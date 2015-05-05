//
//  SRLockServer.cpp
//
//  Created by Yeounoh Chung on 5/1/15 based on IPTradTrxManager.cpp by erfanz.
//

#include "SRLockServer.hpp"
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
#include "../util/RDMACommon.hpp"	// for RDMA verbs


std::list <QueuedRequest> SRLockServer::list_array[ITEM_CNT];
std::mutex SRLockServer::list_mutex[ITEM_CNT];


void* SRLockServer::handle_client(void *param) {
    SRServerContext *ctx = (SRServerContext *) param;
    char temp_char;
    
    // handles client's transactions one by one
    TEST_NZ (start_operation(*ctx));
    
    // once it's finished with the client, it syncs with client to ensure that client is over
    TEST_NZ (sock_sync_data (ctx->sockfd, 1, "W", &temp_char));	// just send a dummy char back and forth
	
    TEST_NZ (ctx->destroy_context());
    return NULL;
}


int SRLockServer::start_operation(SRServerContext &ctx) {
	//start operation for each client connection
	char	temp_char;

	struct timespec firstRequestTime, lastRequestTime;	// for calculating LPMS
	//double avg_lock = 0;

	clock_gettime(CLOCK_REALTIME, &firstRequestTime);	// Fire the  timer

	struct LockRequest req;
	struct LockResponse res;

	ctx.op_num = 0;
	while (ctx.op_num < OPERATIONS_CNT * 2) { // acquire - release pairing
		ctx.op_num = ctx.op_num + 1;
		DEBUG_COUT (std::endl << "[Info] Waiting for operation #" << ctx.op_num/2 << " from client " << ctx.sockfd);
		
		// DEBUG_COUT ("[INFO] Queue/start ------------------------" << ctx.sockfd);
// 		for (int i=0;i<ITEM_CNT;i++){
// 			list_mutex[i].lock();
// 			std::list<QueuedRequest> *list_p = &list_array[i];
// 			std::cout << i << ":   " ;
// 			for (std::list<QueuedRequest>::iterator itr=list_p->begin(); itr != list_p->end();++itr){
// 				std::cout << (*itr).ctx->sockfd << "(" << (*itr).req.request_type << ") " ;
// 			}
// 			std::cout << std::endl;
// 			list_mutex[i].unlock();
// 		}
		

		// ************************************************************************
		// Step 1: Waits for client to post a lock request
		//TEST_NZ (sock_read(ctx.sockfd, (char *)&(req), sizeof(struct LockRequest)));
		TEST_NZ (RDMACommon::poll_completion(ctx.cq_receive));	// Receive LockRequest
		DEBUG_COUT ("[Recv] LockRequest from client" << ctx.sockfd);
		DEBUG_COUT ("+repeat ([Recv] LockRequest from client" << ctx.sockfd << ")");
		
		// ************************************************************************
		// Step 2: Registers client's request (SHARED | EXCLUSIVE | RELEASE)
		// Grant a lock for the front of the queue and (or send a RELEASED message) for client
		TEST_NZ (register_request(ctx));
		
	}

	clock_gettime(CLOCK_REALTIME, &lastRequestTime);	// Fire the  timer
	double micro_elapsed_time = ( ( lastRequestTime.tv_sec - firstRequestTime.tv_sec ) * 1E9 + ( lastRequestTime.tv_nsec - firstRequestTime.tv_nsec ) ) / 1E3;
	//avg_lock /= 1000;

	std::cout << std::endl << "Total Elapsed time (u sec): " << micro_elapsed_time << std::endl;

	//std::cout << "[Stat] Avg lock (u sec):            	" << (double)avg_lock / committed_cnt << std::endl;

	DEBUG_COUT (std::endl << "[Info] Successfully executed all operations of client");

	return 0;
}

int SRLockServer::register_request (SRServerContext &ctx) {
	DEBUG_COUT("+++ Entering register_request");
	if (ctx.lock_request.request_type == LockRequest::RELEASE)
		DEBUG_COUT("+Client " << ctx.sockfd << " lock RELEASE request item " << ctx.lock_request.request_item);
	else
		DEBUG_COUT("+Client " << ctx.sockfd << " lock type " << ctx.lock_request.request_type << " request item " << ctx.lock_request.request_item);
	
	// ctx.lock_request can change in the middle of the following procedures.
	int current_item = ctx.lock_request.request_item;
	int current_lock_mode = ctx.lock_request.request_type;
	
	std::list<QueuedRequest>* list_p = &list_array[ctx.lock_request.request_item];
	if (ctx.lock_request.request_type == LockRequest::RELEASE) {	
		
		list_mutex[ctx.lock_request.request_item].lock();
		DEBUG_COUT("+Client " << ctx.sockfd << " got the lock on item " << ctx.lock_request.request_item);
		
		if (list_p->empty()){
			list_mutex[ctx.lock_request.request_item].unlock();
			DEBUG_COUT("+Client " << ctx.sockfd << " unlock the mutex on item " << ctx.lock_request.request_item);
			
			
			return 1; //error
		}
		else{
			struct QueuedRequest front = list_p->front();
			int list_size = list_p->size();
			
			for (std::list<QueuedRequest>::iterator itr=list_p->begin(); itr != list_p->end();){
				if ((*itr).ctx == &ctx){
					
					bool grant = false;
					
					if((*itr).ctx == front.ctx && list_size > 1){
						DEBUG_COUT("+releasing client at front: " << (*itr).ctx->sockfd);
						grant = true;
					}
					
					ctx.lock_response.response_type = LockResponse::RELEASED;
					
					DEBUG_COUT("+Erasing item " << ctx.lock_request.request_item << " lock for client " << ctx.sockfd);
					
					itr = list_p->erase(itr); //delete causes double free error; erase(itr) takes care of memory.
					list_size = list_p->size(); //update list_size
					DEBUG_COUT("+Erased; current list size: " << list_size << " in client " << ctx.sockfd);	
					
					//this may change the content of ctx.lock_request or req (lock_req_mr)?
					TEST_NZ (RDMACommon::post_RECEIVE(ctx.qp, ctx.lock_req_mr, (uintptr_t)&ctx.lock_request, sizeof(struct LockRequest))); 
					DEBUG_COUT("[Info] receive posted to the queue of " << ctx.sockfd);
					
							
					// TEST_NZ (sock_write(ctx.sockfd, (char *)&(res), sizeof(struct LockResponse)));
					TEST_NZ (RDMACommon::post_SEND(ctx.qp, ctx.lock_res_mr, (uintptr_t)&ctx.lock_response, sizeof(struct LockResponse), true));
					TEST_NZ (RDMACommon::poll_completion(ctx.cq_send));	// Ack for SEND -> this registered a duplicate request from the client
					
					DEBUG_COUT("[Sent] LockResponse RELEASE to client " << ctx.sockfd); // not received
					
					
					// ----------------------------------------------------------------------------------------
					// *** ctx.lock_request can change from this point! (use current_item & current_lock_mode).
					// ----------------------------------------------------------------------------------------
					
					//if released lock was the front, then grant the lock to the next one, if any.
					if(grant){
						DEBUG_COUT("+Granting after releasing the lock for (client,item,mode): " << ctx.sockfd << ", " << current_item << ", " << front.req.request_type);
						
						front = list_p->front(); //update front (previously the second request on the list)
						DEBUG_COUT("+new front of the list to be granted is (client,item,mode): " << front.ctx->sockfd << ", " << front.req.request_item << ", " << front.req.request_type);
						
						int front_current_item = front.ctx->lock_request.request_item;
						int front_current_lock_mode = front.ctx->lock_request.request_type;
						
						//res.response_type = LockResponse::GRANTED;
						front.ctx->lock_response.response_type = LockResponse::GRANTED;
	
						TEST_NZ (RDMACommon::post_RECEIVE(front.ctx->qp, front.ctx->lock_req_mr, (uintptr_t)&front.ctx->lock_request, sizeof(struct LockRequest))); 
						DEBUG_COUT("[Info] RECEIVE posted by client " << ctx.sockfd  << " on behalf of " << front.ctx->sockfd << " to its queue (for item " << front.req.request_item << ")");
						
						//TEST_NZ (sock_write(front.ctx->sockfd, (char *)&(res), sizeof(struct LockResponse)));
						TEST_NZ (RDMACommon::post_SEND(front.ctx->qp, front.ctx->lock_res_mr, (uintptr_t)&front.ctx->lock_response, sizeof(struct LockResponse), true));
						DEBUG_COUT("[Info] SENT posted by client " << ctx.sockfd  << " on behalf of " << front.ctx->sockfd << " to its queue (for item " << front.req.request_item << ")");
						
						TEST_NZ (RDMACommon::poll_completion(front.ctx->cq_send));	// Ack for SEND (false)
						//DEBUG_COUT("[Info] POLL posted by client " << ctx.sockfd  << " on behalf of " << front.ctx->sockfd << " to its queue (for item " << front.req.request_item << ")"); 
						
						// ----------------------------------------------------------------------------------------
						// *** front.ctx.lock_request can change from this point! (use current_item & current_lock_mode).
						// ----------------------------------------------------------------------------------------
						
						
						DEBUG_COUT("[Sent] LockResponse to client " << (front.ctx->sockfd) <<  " (for item " << front_current_item << ")");
						
						
						if (front.req.request_type == LockRequest::SHARED)
							grant_shared_locks(*front.ctx);
					}
					break;
				}
				else
					++itr;
			}
			list_mutex[current_item].unlock();
			DEBUG_COUT("Client " << ctx.sockfd << " unlock the mutex on item " << current_item);
			
			
			return 0;
		}
	}
	else {	// Lock request
		
		DEBUG_COUT("+Registering request from client " << ctx.sockfd << ", (lock mode, item): " << ctx.lock_request.request_type << ", " << ctx.lock_request.request_item);
		QueuedRequest* queued_req = new QueuedRequest;
		queued_req->ctx = &ctx; //SRServerContext
		queued_req->req = ctx.lock_request;
		
		
		
		list_mutex[ctx.lock_request.request_item].lock();
		DEBUG_COUT("+Client " << ctx.sockfd << " got the lock on item " << ctx.lock_request.request_item << " with mode " << ctx.lock_request.request_type);
		
		list_p->push_back(*queued_req);
		
		
		DEBUG_COUT("+Registered; current list size for item " << ctx.lock_request.request_item << ": " << (list_p->size()) << " in client " << ctx.sockfd);
		
		//if registered request is at the front, then grant it the lock.
		if (&ctx == list_p->front().ctx) { 
			
			ctx.lock_response.response_type = LockResponse::GRANTED;
			
			TEST_NZ (RDMACommon::post_RECEIVE(ctx.qp, ctx.lock_req_mr, (uintptr_t)&ctx.lock_request, sizeof(struct LockRequest))); 
			DEBUG_COUT("[Info] RECEIVE posted by the client " << ctx.sockfd << " for self to the queue");
			
			//TEST_NZ (sock_write(front.ctx->sockfd, (char *)&(res), sizeof(struct LockResponse)));
			TEST_NZ (RDMACommon::post_SEND(ctx.qp, ctx.lock_res_mr, (uintptr_t)&ctx.lock_response, sizeof(struct LockResponse), true));
			DEBUG_COUT("[Info] SENT posted by the client " << ctx.sockfd << " for self to the queue");
			
			TEST_NZ (RDMACommon::poll_completion(ctx.cq_send));	// Ack for SEND
			DEBUG_COUT("[Info] POLL posted by the client " << ctx.sockfd << " for self (" << ctx.sockfd << ") to the queue");
			
			// ----------------------------------------------------------------------------------------
			// *** ctx.lock_request can change from this point! (use current_item & current_lock_mode).
			// ----------------------------------------------------------------------------------------
			
			DEBUG_COUT("[Sent] LockResponse " << current_lock_mode << " to client " << (ctx.sockfd));
			
			if(ctx.lock_request.request_type == LockRequest::SHARED)
				grant_shared_locks(ctx);
		}
		else {
			DEBUG_COUT("Registered Item " << current_item << " lock is not granted, for client " << ctx.sockfd);
		}
		list_mutex[current_item].unlock();
		DEBUG_COUT("+Client " << ctx.sockfd << " unlock the mutex on item " << current_item);
		
		
		return 0;
	}
	
	return 1;
}

int SRLockServer::grant_shared_locks (SRServerContext &ctx) {
	
	std::list<QueuedRequest>* list_p = &list_array[ctx.lock_request.request_item];
	
	if (ctx.lock_request.request_type == LockRequest::RELEASE) {
		return 1; //error
	} //else, queue should not be empty
	else { //grant lock to the front request in the queue (this may not be ctx)
		ctx.lock_response.response_type = LockResponse::GRANTED;
				
		struct QueuedRequest front = list_p->front();
		
		if (list_p->size() > 1 && front.req.request_type == LockRequest::SHARED){
			DEBUG_COUT("+Granting other shared locks on item " << ctx.lock_request.request_item);
			std::list<QueuedRequest>::iterator itr=list_p->begin();
			++itr; //skip front
			for (; itr != list_p->end();++itr){
				int current_item = (*itr).req.request_item;
				int current_lock_mode = (*itr).req.request_type;
				
				if ((*itr).req.request_type == LockRequest::EXCLUSIVE){
					DEBUG_COUT("+Encounted EXCLUSIVE");
					break;
				}
				else {
					TEST_NZ (RDMACommon::post_RECEIVE((*itr).ctx->qp, (*itr).ctx->lock_req_mr, (uintptr_t)&(*itr).ctx->lock_request, sizeof(struct LockRequest))); 
					DEBUG_COUT("[Info] RECEIVE posted for client " << (*itr).ctx->sockfd << " to the queue (in grant_shared_locks)");
					
					//TEST_NZ (sock_write((*itr).ctx->sockfd, (char *)&(res), sizeof(struct LockResponse)));
					TEST_NZ (RDMACommon::post_SEND((*itr).ctx->qp, (*itr).ctx->lock_res_mr, (uintptr_t)&(*itr).ctx->lock_response, sizeof(struct LockResponse), true));
					DEBUG_COUT("[Info] SEND posted for  client " << (*itr).ctx->sockfd << " to the queue (in grant_shared_locks)");
					TEST_NZ (RDMACommon::poll_completion((*itr).ctx->cq_send));	// Ack for SEND
					//DEBUG_COUT("[Info] POLL posted by the client " << ctx.sockfd << " for  client " << (*itr).ctx->sockfd << " to the queue");
					
					DEBUG_COUT("+[Sent] LockResponse " << (*itr).req.request_type << " to client " << ((*itr).ctx->sockfd));
				}
			}
			DEBUG_COUT("+done.");
		}
		
		return 0;
	}
	
	return 1;
}

int SRLockServer::start_server () {
	struct sockaddr_in serv_addr, returned_addr;
	socklen_t len = sizeof(returned_addr);
	pthread_t master_threads[CLIENTS_CNT];
	struct SRServerContext ctx[CLIENTS_CNT]; //server context for each client
	char temp_char;
    
		
	// Call socket(), bind() and listen()
	TEST_NZ (server_socket_setup(&server_sockfd, CLIENTS_CNT));
    
	// accept connections from clients
	std::cout << std::endl << "[Info] Waiting for " << CLIENTS_CNT << " client(s) on port " << SERVER_TCP_PORT << std::endl;	
	
	for (int c = 0; c < CLIENTS_CNT; c++){
		ctx[c].sockfd = accept (server_sockfd, (struct sockaddr *) &returned_addr, &len);
		if (ctx[c].sockfd < 0) {
			std::cerr << "ERROR on accept() client " << c << std::endl;
			return -1;
		}
		//ctx[c].client_ip = "";
		//ctx[c].client_ip	+= std::string(inet_ntoa (returned_addr.sin_addr));
		//ctx[c].client_port	= (int) ntohs(returned_addr.sin_port);
		std::cout << "[Conn] Received client " << c << " (" << ctx[c].client_ip << ", " << ctx[c].client_port << ") on sock " << ctx[c].sockfd << std::endl;	
		
		ctx[c].ib_port = SERVER_IB_PORT;
		TEST_NZ (ctx[c].create_context());
		DEBUG_COUT("[Info] Context for client " << c << " created");
		
		// Before connecting the queuepairs, the lock server must post RECEIVE 
		TEST_NZ (RDMACommon::post_RECEIVE(ctx[c].qp, ctx[c].lock_req_mr, (uintptr_t)&ctx[c].lock_request, sizeof(struct LockRequest)));
		DEBUG_COUT("[Info] Receive posted");
		
		
		// connect the QPs
		TEST_NZ (RDMACommon::connect_qp (&(ctx[c].qp), ctx[c].ib_port, ctx[c].port_attr.lid, ctx[c].sockfd));	
		DEBUG_COUT("[Conn] QPed to client " << c);	
	}
    
	std::cout << "[Info] Established connection to all " << CLIENTS_CNT << " client(s)." << std::endl; 
    
    
	for (int c = 0; c < CLIENTS_CNT; c++) {
		pthread_create(&master_threads[c], NULL, SRLockServer::handle_client, &ctx[c]);
	}
    
	//wait for handlers to finish
	for (int i = 0; i < CLIENTS_CNT; i++) {
		pthread_join(master_threads[i], NULL);
	}
    
	// close server socket
	TEST_NZ (destroy_resources());
	std::cout << "[Info] Server is done and destroyed its resources!" << std::endl;
}

int SRLockServer::destroy_resources () {
	close(server_sockfd);	// close the socket
	return 0;
}

void SRLockServer::usage (const char *argv0) {
	std::cout << "Usage:" << std::endl;
	std::cout << "[IPTradTrxManager | IBTradTrxManager] starts a server and wait for connection" << std::endl;
}

int main (int argc, char *argv[]) {
    if (argc != 1) {
        SRLockServer::usage(argv[0]);
        return 1;
    }
    SRLockServer server;
    server.start_server();
    return 0;
}
