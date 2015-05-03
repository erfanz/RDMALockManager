//
//  LockServer.cpp
//
//  Created by Yeounoh Chung on 5/1/15 based on IPTradTrxManager.cpp by erfanz.
//

#include "LockServer.h"
#include "../util/utils.hpp"
#include <stdio.h>
#include <string.h>
#include <sstream>	// for stringstream
#include <unistd.h>
#include <inttypes.h>
#include <endian.h>
#include <getopt.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <iostream>
#include <netinet/tcp.h>	// for setsockopt

std::list <QueuedRequest> LockServer::list_array[ITEM_CNT];

void* LockServer::handle_client(void *param) {
    ServerContext *ctx = (ServerContext *) param;
    char temp_char;
    
    // handles client's transactions one by one
    TEST_NZ (start_operation(*ctx));
    
    // once it's finished with the client, it syncs with client to ensure that client is over
    TEST_NZ (sock_sync_data (ctx->client_ctx.sockfd, 1, "W", &temp_char));	// just send a dummy char back and forth
	
    TEST_NZ (ctx->destroy_context());
    return NULL;
}


int LockServer::start_operation(ServerContext &ctx) {
	//start operation for each client connection
	char	temp_char;

	struct timespec firstRequestTime, lastRequestTime;	// for calculating LPMS
	//double avg_lock = 0;

	clock_gettime(CLOCK_REALTIME, &firstRequestTime);	// Fire the  timer

	struct LockRequest req;
	struct LockResponse res;

	ctx.client_ctx.op_num = 0;
	while (ctx.client_ctx.op_num < OPERATIONS_CNT * 2) {
		ctx.client_ctx.op_num = ctx.client_ctx.op_num + 1;
		DEBUG_COUT (std::endl << "[Info] Waiting for operation #" << ctx.client_ctx.op_num);


		// ************************************************************************
		// Step 1: Waits for client to post a lock request
		TEST_NZ (sock_read(ctx.client_ctx.sockfd, (char *)&(req), sizeof(struct LockRequest)));
		DEBUG_COUT ("[Recv] LockRequest from client");
		
		
		// ************************************************************************
		// Step 2: Registers client's request (SHARED | EXCLUSIVE | RELEASE)
		TEST_NZ (register_request(ctx,req));
		
		
		// ************************************************************************
		// Step 3: Grant a lock for the front of the queue and (or send a RELEASED message) for client
		TEST_NZ (grant_request(ctx,req,res));
		
	}

	clock_gettime(CLOCK_REALTIME, &lastRequestTime);	// Fire the  timer
	double micro_elapsed_time = ( ( lastRequestTime.tv_sec - firstRequestTime.tv_sec ) * 1E9 + ( lastRequestTime.tv_nsec - firstRequestTime.tv_nsec ) ) / 1000;
	//avg_lock /= 1000;

	std::cout << std::endl << "Total Elapsed time (u sec): " << micro_elapsed_time << std::endl;

	//std::cout << "[Stat] Avg lock (u sec):            	" << (double)avg_lock / committed_cnt << std::endl;

	DEBUG_COUT (std::endl << "[Info] Successfully executed all operations of client");

	return 0;
}

int LockServer::register_request (ServerContext &ctx, struct LockRequest &req) {
	
	DEBUG_COUT("[Info] Client lock type " << req.request_type << " request item: " << req.request_item);
	std::list<QueuedRequest>* list_p = &list_array[req.request_item];
	
	if (req.request_type == LockRequest::RELEASE) {	
		if (list_p->empty()){
			return 1; //error
		}
		else{
			//struct QueuedRequest front = queue.front();
			//delete &front;
			for (std::list<QueuedRequest>::iterator itr=list_p->begin(); itr != list_p->end();){
				if ((*itr).ctx == &ctx)
					itr = list_p->erase(itr);
				else
					++itr;
			}
			
			return 0;
		}
	}
	else {
		QueuedRequest* queued_req = new QueuedRequest;
		queued_req->ctx = &ctx;
		queued_req->req = req;
		
		list_p->push_back(*queued_req); //store the object itself, and no need to delete
		
		return 0;
	}
	
	return 1;
}

int LockServer::grant_request (ServerContext &ctx, struct LockRequest &req, struct LockResponse &res) {
	std::list<QueuedRequest>* list_p = &list_array[req.request_item];
	
	if (req.request_type == LockRequest::RELEASE) {
		//RELEASE requests always succeed; send to ctx.client_ctx
		res.response_type = LockResponse::RELEASED;
		TEST_NZ (sock_write(ctx.client_ctx.sockfd, (char *)&(res), sizeof(struct LockResponse)));
		DEBUG_COUT("[Sent] LockResponse RELEASE to client");
		
		return 0;
	} //else, queue should not be empty
	else { //grant lock to the front request in the queue (this may not be ctx.client_ctx)
		res.response_type = LockResponse::GRANTED;
		struct QueuedRequest front = list_p->front();
		TEST_NZ (sock_write(front.ctx->client_ctx.sockfd, (char *)&(res), sizeof(struct LockResponse)));
		DEBUG_COUT("[Sent] LockResponse " << front.req.request_type << " to client " << list_p->size());
		
		if (front.req.request_type == LockRequest::SHARED){
			std::list<QueuedRequest>::iterator itr=list_p->begin();
			++itr;
			for (; itr != list_p->end();++itr){
				if ((*itr).req.request_type == LockRequest::EXCLUSIVE)
					break;
				else {
					TEST_NZ (sock_write((*itr).ctx->client_ctx.sockfd, (char *)&(res), sizeof(struct LockResponse)));
					DEBUG_COUT("[Sent] LockResponse " << (*itr).req.request_type << " to client");
				}
			}
		}
		
		return 0;
	}
	
	return 1;
}

int LockServer::start_server () {
	struct sockaddr_in serv_addr, returned_addr;
	socklen_t len = sizeof(returned_addr);
	pthread_t master_threads[CLIENTS_CNT];
	struct ServerContext ctx[CLIENTS_CNT]; //server context for each client
	char temp_char;
    
	// Open Socket
	server_sockfd = socket (AF_INET, SOCK_STREAM, 0);
	if (server_sockfd < 0) {
		std::cerr << "Error opening server socket" << std::endl;
		return -1;
	}
    
	int flag = 1;
	int opt_return = setsockopt(server_sockfd, IPPROTO_TCP, TCP_NODELAY, (char *) &flag, sizeof(int)); //TCP socket
	if (opt_return < 0) {
		std::cerr << "Could not set server socket options" << std::endl;
		return -1;
	}
    
	// Bind
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(SERVER_TCP_PORT);
	TEST_NZ(bind(server_sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)));
    
	// listen
	TEST_NZ(listen (server_sockfd, CLIENTS_CNT*1 + CLIENTS_CNT)); //SERVER_CNT = 1
    
	// accept connections from clients
	std::cout << std::endl << "[Info] Waiting for " << CLIENTS_CNT << " client(s) on port " << SERVER_TCP_PORT << std::endl;	
	for (int c = 0; c < CLIENTS_CNT; c++){
		ctx[c].client_ctx.sockfd = accept (server_sockfd, (struct sockaddr *) &returned_addr, &len);
		if (ctx[c].client_ctx.sockfd < 0) {
			std::cerr << "ERROR on accept() client " << c << std::endl;
			return -1;
		}
		ctx[c].client_ctx.client_ip = "";
		ctx[c].client_ctx.client_ip	+= std::string(inet_ntoa (returned_addr.sin_addr));
		ctx[c].client_ctx.client_port	= (int) ntohs(returned_addr.sin_port);
		std::cout << "[Conn] Received client " << c << " (" << ctx[c].client_ctx.client_ip << ", " << ctx[c].client_ctx.client_port << ") on sock " << ctx[c].client_ctx.sockfd << std::endl;		
	}
    
	std::cout << "[Info] Established connection to all " << CLIENTS_CNT << " client(s)." << std::endl; 
    
    
	for (int c = 0; c < CLIENTS_CNT; c++) {
		pthread_create(&master_threads[c], NULL, LockServer::handle_client, &ctx[c]);
	}
    
	//wait for handlers to finish
	for (int i = 0; i < CLIENTS_CNT; i++) {
		pthread_join(master_threads[i], NULL);
	}
    
	// close server socket
	TEST_NZ (destroy_resources());
	std::cout << "[Info] Server is done and destroyed its resources!" << std::endl;
}

int LockServer::destroy_resources () {
	close(server_sockfd);	// close the socket
	return 0;
}

void LockServer::usage (const char *argv0) {
	std::cout << "Usage:" << std::endl;
	std::cout << "[IPTradTrxManager | IBTradTrxManager] starts a server and wait for connection" << std::endl;
}

int main (int argc, char *argv[]) {
    if (argc != 1) {
        LockServer::usage(argv[0]);
        return 1;
    }
    LockServer server;
    server.start_server();
    return 0;
}
