/*
*	BClientCentricServer.cpp
*
*	Created on: 25.Jan.2015
*	Author: erfanz
*/

#include "BClientCentricServer.hpp"
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
#include <sys/resource.h>	// getrusage()


BClientCentricServer::~BClientCentricServer () {
	delete[](locks);
	close(server_sockfd);
}

int BClientCentricServer::initialize_data_structures(){
	locks = new uint64_t[ITEM_CNT];
	for (int i = 0; i < ITEM_CNT; i++)
		locks[i] = 0;
	return 0;
}

int BClientCentricServer::initialize_context(BServerContext &ctx) {
	ctx.ib_port	= ib_port;
	ctx.locks	= locks;
	return 0;
}

void BClientCentricServer::usage (const char *argv0) {
	std::cout << "Usage:" << std::endl;
	std::cout << argv0 << std::endl;
	std::cout << "starts a server and waits for connection on port Config.SERVER_TCP_PORT" << std::endl;
}

void* BClientCentricServer::handle_client(void *param) {
	BServerContext *ctx = (BServerContext *) param;
	char temp_char;
	
	DEBUG_COUT("[in handle client]");
	
	// TEST_NZ (RDMACommon::post_RECEIVE(ctx->qp, ctx->recv_data_mr, (uintptr_t)&ctx->recv_data_msg, sizeof(int)));
	TEST_NZ (sock_sync_data (ctx->sockfd, 1, "W", &temp_char));	// just send a dummy char back and forth
	
	DEBUG_COUT("[Synced with client]");
	
	/*
	int iteration = 0;
	while (iteration < OPERATIONS_CNT) {
	TEST_NZ (RDMACommon::poll_completion(ctx->cq));
	DEBUG_COUT("[Recv] request from client");
		
	TEST_NZ (RDMACommon::post_RECEIVE(ctx->qp, ctx->recv_data_mr, (uintptr_t)&ctx->recv_data_msg, sizeof(int)));	// for 
	DEBUG_COUT("[Info] receive posted to the queue");
		
	if (iteration % 1000 == 0) {
	TEST_NZ (RDMACommon::post_SEND(ctx->qp, ctx->send_data_mr, (uintptr_t)ctx->send_data_msg, 10 * sizeof(char), true));
	DEBUG_COUT("[Sent] response to client");
		
	TEST_NZ (RDMACommon::poll_completion(ctx->cq));	// for SEND
	DEBUG_COUT("[Info] completion received");
		
	}
	else {
	TEST_NZ (RDMACommon::post_SEND(ctx->qp, ctx->send_data_mr, (uintptr_t)ctx->send_data_msg, 10 * sizeof(char), false));
	DEBUG_COUT("[Sent] response to client (without completion)");
	}		
	iteration++;
	}
	*/
	DEBUG_COUT("[Sent] buffer info to client");
}


int BClientCentricServer::start_server () {	
	tcp_port	= SERVER_TCP_PORT;
	ib_port		= SERVER_IB_PORT;
	BServerContext ctx[CLIENTS_CNT];
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
		ctx[i].sockfd  = accept (server_sockfd, (struct sockaddr *) &cli_addr, &clilen);
		if (ctx[i].sockfd < 0){
			std::cerr << "ERROR on accept" << std::endl;
			return -1;
		}
		std::cout << "[Conn] Received client #" << i << " on socket " << ctx[i].sockfd << std::endl;
	
		// create all resources
		TEST_NZ (ctx[i].create_context());
		DEBUG_COUT("[Info] Context for client " << i << " created");
		
		// connect the QPs
		TEST_NZ (RDMACommon::connect_qp (&(ctx[i].qp), ctx[i].ib_port, ctx[i].port_attr.lid, ctx[i].sockfd));
		DEBUG_COUT("[Conn] QPed to client " << i);
	
		// prepare server buffer with read message
		memcpy(&(ctx[i].send_message_msg.peer_mr), ctx[i].locks_mr, sizeof(struct ibv_mr));
		ctx[i].send_message_msg.client_id =  static_cast<uint32_t>(i + 1);
		
	}
	
	for (int i = 0; i < CLIENTS_CNT; i++){
		// send memory locations using SEND 
		TEST_NZ (RDMACommon::post_SEND (ctx[i].qp, ctx[i].send_message_mr, (uintptr_t)&ctx[i].send_message_msg, sizeof(struct BMemoryKeys), true));
		TEST_NZ (RDMACommon::poll_completion(ctx[i].cq));
		DEBUG_COUT("[Sent] buffer info to client " << i);
	}
	
	// Server waits for the client to muck with its memory
	
	struct timespec firstRequestTime, lastRequestTime;	// for calculating LPMS
    
	struct rusage usage;
	struct timeval start_user_usage, start_kernel_usage, end_user_usage, end_kernel_usage;
	clock_gettime(CLOCK_REALTIME, &firstRequestTime);	// Fire the  timer
	getrusage(RUSAGE_SELF, &usage);
	start_kernel_usage = usage.ru_stime;
	start_user_usage = usage.ru_utime;
	
	for (int i = 0; i < CLIENTS_CNT; i++) {
		TEST_NZ (sock_sync_data (ctx[i].sockfd, 1, "W", &temp_char));	// just send a dummy char back and forth
		DEBUG_COUT("[Conn] Client " << i << " notified it's finished");
		TEST_NZ (ctx[i].destroy_context());
		std::cout << "[Info] Destroying client " << i << " resources" << std::endl;
	}
	
	getrusage(RUSAGE_SELF, &usage);
	clock_gettime(CLOCK_REALTIME, &lastRequestTime);	// Fire the  timer
	
	end_user_usage = usage.ru_utime;
	end_kernel_usage = usage.ru_stime;
	
	
	double user_cpu_microtime = ( end_user_usage.tv_sec - start_user_usage.tv_sec ) * 1E6 + ( end_user_usage.tv_usec - start_user_usage.tv_usec );
	double kernel_cpu_microtime = ( end_kernel_usage.tv_sec - start_kernel_usage.tv_sec ) * 1E6 + ( end_kernel_usage.tv_usec - start_kernel_usage.tv_usec );
	
	double micro_elapsed_time = ( ( lastRequestTime.tv_sec - firstRequestTime.tv_sec ) * 1E6 + ( lastRequestTime.tv_nsec - firstRequestTime.tv_nsec )/ 1E3 );
	std::cout << std::endl << "[Stat] Avg Elapsed time per operation (u sec): " << micro_elapsed_time / OPERATIONS_CNT / CLIENTS_CNT << std::endl;
	std::cout << "[Stat] Avg kernel time per operation (u sec): " << kernel_cpu_microtime / OPERATIONS_CNT / CLIENTS_CNT << std::endl;
	std::cout << "[Stat] Avg user time per operation (u sec): " << user_cpu_microtime / OPERATIONS_CNT / CLIENTS_CNT << std::endl;
	std::cout << "[Stat] CPU utilization: " << (user_cpu_microtime + kernel_cpu_microtime) / micro_elapsed_time << std::endl;

	
	
	std::cout << "[Info] Server's ready to gracefully get destroyed" << std::endl;	
}

int main (int argc, char *argv[]) {
	if (argc != 1) {
		BClientCentricServer::usage(argv[0]);
		return 1;
	}
	BClientCentricServer server;
	server.start_server();
	return 0;
}