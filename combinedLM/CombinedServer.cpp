/*
 *	CombinedServer.cpp
 *
 *	Created on: 25.Jan.2015
 *	Author: erfanz
 */

#include "CombinedServer.hpp"
#include "../config.hpp"
#include "../util/utils.hpp"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>
#include <endian.h>
#include <getopt.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <iostream>
#include <vector>


void CombinedServer::convert(uint64_t x) {
    std::vector<int> ret;
     uint64_t one = (uint64_t) 1;
  
     int bit = 0;
     while(bit < 64) {
       if (x & one)
         ret.push_back(1);
       else
         ret.push_back(0);
       x>>=1;  
       bit++;
     }
     for (int i=0; i <64  ; i++) {
     	if (i%4 == 0 && i!=0)
     		std::cout << " ";
     	std::cout << ret.at(63-i);
     }
     std::cout << std::endl;
}

CombinedServer::~CombinedServer () {
	delete[](outstanding_array);
	close(server_sockfd);
}

int CombinedServer::initialize_data_structures(){
	outstanding_array = new LockRequest[CLIENTS_CNT];
	
	for (int i = 0; i < ITEM_CNT; i++)
		item_lock_array[i] = 0;

	for (int i = 0; i < CLIENTS_CNT; i++)
		outstanding_array[i].request_type = LockRequest::EMPTY;
		
	return 0;
}

int CombinedServer::initialize_context(CombinedServerContext &ctx) {
	ctx.ib_port	= ib_port;
	ctx.item_lock_array	= item_lock_array;
	ctx.outstanding_array = outstanding_array;
	return 0;
}

void CombinedServer::usage (const char *argv0) {
	std::cout << "Usage:" << std::endl;
	std::cout << argv0 << std::endl;
	std::cout << "starts a server and waits for connection on port Config.SERVER_TCP_PORT" << std::endl;
}

int CombinedServer::handle_shared_lock(CombinedServerContext &ctx){
	uint32_t client_id = ctx.client_id;
	int index_in_outstanding_array = client_id - 1;
	int item_id = outstanding_array[index_in_outstanding_array].request_item;

	// DEBUG_COUT("[Info] Found outstanding SHARED lock on item " << item_id << " from client " << client_id);
	DEBUG_COUT("[Info] head layout:");
	for (int i = 0; i < ITEM_CNT; i++) {
		DEBUG_COUT("-- item " << i << ": " <<  (item_lock_array[i] >> 32) << "|" << (item_lock_array[i] << 32 >> 32));
	}

	// !!!!! MUST BE ATOMIC !!!!!!!
	//uint32_t exclusive_part = (uint32_t) (item_lock_array[item_id] >> 32);
	uint64_t arg = (uint64_t)1 << 63;			// 1000 0000 0000 0000
	uint32_t arg2 = (uint32_t)1;				// 0000 0000 0000 0001
	uint64_t arg3 = (uint64_t)4294967295;		// 0000 0000 FFFF FFFF
	
	// Let's say lock status is 0XXX XXXX SSSS SSSS
	
	convert(item_lock_array[item_id]);
	uint64_t original_item_lock = atomic_fetch_or(&item_lock_array[item_id], arg);
	convert(item_lock_array[item_id]);
	
	uint32_t exclusive_part = (uint32_t) (original_item_lock >> 32);
	convert(exclusive_part);
	 
	// check if the lock is exlusively owned by another client
	if (exclusive_part == 0) {
		// so the lock is 0000 0000 SSSS SSSS
		
		// First, the server cleans the request
		outstanding_array[index_in_outstanding_array].request_type = LockRequest::EMPTY;
		
		atomic_fetch_and(&item_lock_array[item_id], arg3);	// changing back the lock from 1000 0000 SSSS SSSS to 0000 0000 SSSS SSSS

		ctx.lock_response.response_type = LockResponse::GRANTED;

		TEST_NZ (RDMACommon::post_SEND(ctx.qp, ctx.lock_res_mr, (uintptr_t)&ctx.lock_response, sizeof(struct LockResponse), true));

		TEST_NZ (RDMACommon::poll_completion(ctx.cq));	// Ack for SEND
		DEBUG_COUT("[SEND] Grant message to client " << client_id << " on item " << item_id);
	}
	else {
		// so the lock was originally 0XXX XXXX SSSS SSSS
		uint64_t new_position = ((uint64_t) exclusive_part) << 32;
		convert(new_position);
		atomic_fetch_or(&item_lock_array[item_id], new_position);	// changing back the lock from 1XXX XXXX SSSS SSSS to 0XXX XXXX SSSS SSSS
	}
	return 0;
}

int CombinedServer::handle_exclusive_lock(CombinedServerContext &ctx){	
	uint32_t client_id = ctx.client_id;
	int index_in_outstanding_array = client_id - 1;

	int item_id = outstanding_array[index_in_outstanding_array].request_item;

	// DEBUG_COUT("[Info] Found outstanding EXCLUSIVE lock on item " << item_id << " from client " << client_id);
	DEBUG_COUT("[Info] head layout:");
	for (int i = 0; i < ITEM_CNT; i++) {
		DEBUG_COUT("-- item " << i << ": " <<  (item_lock_array[i] >> 32) << "|" << (item_lock_array[i] << 32 >> 32));
	}


	// check if the lock is owned (SHARED or EXCLUSIVE) by another client
	if (item_lock_array[item_id] == 0) {
		// First, the server cleans the request from the outstanding array
		outstanding_array[index_in_outstanding_array].request_type = LockRequest::EMPTY;

		DEBUG_COUT("[Info] Old value of lock_array: (" << (item_lock_array[item_id] >> 32) << "|" << (item_lock_array[item_id] << 32 >> 32) << ")");


		// !!!!! MUST BE ATOMIC !!!!!!!
		item_lock_array[item_id] = (uint64_t) client_id << 32 | 0;

		DEBUG_COUT("[Info] New value of lock_array: (" << (item_lock_array[item_id] >> 32) << "|" << (item_lock_array[item_id] << 32 >> 32) << ")");

		ctx.lock_response.response_type = LockResponse::GRANTED;

		TEST_NZ (RDMACommon::post_SEND(ctx.qp, ctx.lock_res_mr, (uintptr_t)&ctx.lock_response, sizeof(struct LockResponse), true));

		TEST_NZ (RDMACommon::poll_completion(ctx.cq));	// Ack for SEND
		DEBUG_COUT("[SEND] Grant message to client " << client_id << " on item " << item_id);
	}
	return 0;
	
}

int CombinedServer::handle_release_lock(CombinedServerContext &ctx){
	uint32_t client_id = ctx.client_id;
	int index_in_outstanding_array = client_id - 1;
	int item_id = outstanding_array[index_in_outstanding_array].request_item;
	int lock_mode = outstanding_array[index_in_outstanding_array].request_type;

	// First, the server cleans the request from the outstanding array
	outstanding_array[index_in_outstanding_array].request_type = LockRequest::EMPTY;

	if (lock_mode == LockRequest::RELEASE_SHARED) {
		DEBUG_COUT("[Info] Found lock request type RELEASE_SHARED on item " << item_id << " from client " << client_id);

		// !!!!! MUST BE ATOMIC !!!!!!!
		// item_lock_array[item_id] -= 1;
		uint64_t return_r = atomic_fetch_sub(&item_lock_array[item_id], (uint64_t) 1);
		convert(return_r);
		
		DEBUG_COUT("[Info] New value of lock_array: (" << (item_lock_array[item_id] >> 32) << "|" << (item_lock_array[item_id] << 32 >> 32) << ")");
	}
	else {
		// ASSERT lock_mode == LockRequest::RELEASE_EXCLUSIVE
		DEBUG_COUT("[Info] Found lock request type RELEASE_EXCLUSIVE on item " << item_id << " from client " << client_id);

	

		// !!!!! MUST BE ATOMIC !!!!!!!
		//item_lock_array[item_id] = new_lock_value;
		// uint64_t new_lock_value = item_lock_array[item_id] << 32 >> 32;
		uint64_t arg3 = (uint64_t)4294967295;		// 0000 0000 FFFF FFFF
		uint64_t return_r = atomic_fetch_and(&item_lock_array[item_id], arg3);
		convert(return_r);
		// DEBUG_COUT("[Info] head layout:");
		// for (int i = 0; i < ITEM_CNT; i++) {
		// 	DEBUG_COUT("-- item " << i << ": " <<  (item_lock_array[i] >> 32) << "|" << (item_lock_array[i] << 32 >> 32));
		// }
		//sleep(0.5);
	}

	ctx.lock_response.response_type = LockResponse::RELEASED;
	//sleep(3);

	TEST_NZ (RDMACommon::post_SEND(ctx.qp, ctx.lock_res_mr, (uintptr_t)&ctx.lock_response, sizeof(struct LockResponse), true));
	TEST_NZ (RDMACommon::poll_completion(ctx.cq));	// Ack for SEND

	DEBUG_COUT("[SEND] RELEASED message to client " << client_id << " on item " << item_id);
	return 0;
	
}


int CombinedServer::start_operations(CombinedServerContext *ctx){
	int lock_release_request_cnt = 0;
	int shared_lock_request_cnt = 0;
	int exclusive_lock_request_cnt = 0;
	// since the server knows that it will get a release request per operation, we can use this knowledge to know when to kill the server 
	while(lock_release_request_cnt < OPERATIONS_CNT * CLIENTS_CNT) {
		for (int i = 0; i < CLIENTS_CNT; i++) {
			if (outstanding_array[i].request_type != LockRequest::EMPTY) {
				// DEBUG_COUT("[Info] outstanding layout:");
				// for (int c= 0; c < CLIENTS_CNT; c++) {
				// 	DEBUG_COUT("********* outstanding client " << c << ": " <<  (outstanding_array[c].request_item));
				// }
				
				if (outstanding_array[i].request_type == LockRequest::SHARED){
					shared_lock_request_cnt += 1;
					TEST_NZ (handle_shared_lock(ctx[i]));
				}
				else if (outstanding_array[i].request_type == LockRequest::EXCLUSIVE){
					exclusive_lock_request_cnt += 1;
					TEST_NZ (handle_exclusive_lock(ctx[i]));
				}
				else if (outstanding_array[i].request_type == LockRequest::RELEASE_SHARED
				|| outstanding_array[i].request_type == LockRequest::RELEASE_EXCLUSIVE) {
					lock_release_request_cnt += 1;
					TEST_NZ (handle_release_lock(ctx[i]));
				}
				else {
					std::cerr << "unrecognized lock type" << std::endl;
					return -1;
				}
			}
		}
	}
	
	std::cout << "Shared lock requests in the outstanding: " << shared_lock_request_cnt << std::endl;
	std::cout << "Exclusive lock requests in the outstanding: " << exclusive_lock_request_cnt << std::endl;
	
	
	return 0;
}



int CombinedServer::start_server () {	
	tcp_port	= SERVER_TCP_PORT;
	ib_port		= SERVER_IB_PORT;
	CombinedServerContext ctx[CLIENTS_CNT];
	struct sockaddr_in serv_addr, cli_addr;
	socklen_t clilen = sizeof(cli_addr);
	char temp_char;
	
	
	TEST_NZ(initialize_data_structures());

	std::cout << "[Info] Server is waiting for " << CLIENTS_CNT
		<< " client(s) on tcp port: " << tcp_port << ", ib port: " << ib_port << std::endl;
	
	// Open Socket
	server_sockfd = socket (AF_INET, SOCK_STREAM, 0);
	if (server_sockfd < 0) {
		std::cerr << "Error opening socket" << std::endl;
		return -1;
	}
	
	// Bind
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(tcp_port);
	TEST_NZ(bind(server_sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)));
	

	// listen				  
	TEST_NZ(listen (server_sockfd, CLIENTS_CNT));
	
	
	// accept connections
	for (int i = 0; i < CLIENTS_CNT; i++){
		initialize_context(ctx[i]);
		ctx[i].client_id = static_cast<uint32_t>(i + 1);;
		ctx[i].sockfd  = accept (server_sockfd, (struct sockaddr *) &cli_addr, &clilen);
		if (ctx[i].sockfd < 0){
			std::cerr << "ERROR on accept" << std::endl;
			return -1;
		}
		std::cout << "[Conn] Received client #" << ctx[i].client_id << " on socket " << ctx[i].sockfd << std::endl;
	
		// create all resources
		TEST_NZ (ctx[i].create_context());
		DEBUG_COUT("[Info] Context for client " << ctx[i].client_id << " created");
		
		// connect the QPs
		TEST_NZ (RDMACommon::connect_qp (&(ctx[i].qp), ctx[i].ib_port, ctx[i].port_attr.lid, ctx[i].sockfd));
		DEBUG_COUT("[Conn] QPed to client " << ctx[i].client_id);
	
		// prepare server buffer with read message
		memcpy(&(ctx[i].send_message_msg.peer_item_lock_mr),			ctx[i].item_lock_mr,	sizeof(struct ibv_mr));
		memcpy(&(ctx[i].send_message_msg.peer_outstanding_mr), ctx[i].outstanding_mr, sizeof(struct ibv_mr));
		ctx[i].send_message_msg.client_id =  ctx[i].client_id;
	}
	
	for (int i = 0; i < CLIENTS_CNT; i++){
		// send memory locations using SEND 
		TEST_NZ (RDMACommon::post_SEND (ctx[i].qp, ctx[i].send_message_mr, (uintptr_t)&ctx[i].send_message_msg, sizeof(struct CombinedMemoryKeys), true));
		TEST_NZ (RDMACommon::poll_completion(ctx[i].cq));
		DEBUG_COUT("[SEND] buffer info to client " << ctx[i].client_id);
	}
	
	TEST_NZ(start_operations(ctx));
	
	for (int i = 0; i < CLIENTS_CNT; i++) {
		TEST_NZ (sock_sync_data (ctx[i].sockfd, 1, "W", &temp_char));	// just send a dummy char back and forth
		DEBUG_COUT("[Conn] Client " << ctx[i].client_id << " notified it's finished");
		TEST_NZ (ctx[i].destroy_context());
		std::cout << "[Info] Destroying client " << i << " resources" << std::endl;
	}	
	std::cout << "[Info] Server's ready to gracefully get destroyed" << std::endl;	
}

int main (int argc, char *argv[]) {
	if (argc != 1) {
		CombinedServer::usage(argv[0]);
		return 1;
	}
	CombinedServer server;
	server.start_server();
	return 0;
}