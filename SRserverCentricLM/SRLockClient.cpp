/*
 *	SRLockClient.cpp
 *	serverCentricLM
 *
 * created by yeounoh on 5/1/2015.
 */

#include "SRLockClient.hpp"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>
#include <endian.h>
#include <getopt.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <cstdlib>
#include <netdb.h>
#include <iostream>
#include <time.h>	// for struct timespec
#include "../util/RDMACommon.hpp"	// for RDMA verbs


int SRLockClient::start_operation (SRClientContext &ctx) {
	
	struct timespec firstRequestTime, lastRequestTime, beforeAcquisition, afterAcquisition, beforeRelease, afterRelease;
	char temp_char;

	

	struct LockRequest req_cpy;
	
	double sumExclusiveAcqTime= 0.0, sumSharedAcqTime= 0.0, sumExclusiveRelTime= 0.0, sumSharedRelime= 0.0;
	double acquisitionTime, releaseTime;
	int exclusiveCount= 0, sharedCount= 0;
	
	clock_gettime(CLOCK_REALTIME, &firstRequestTime); // Fire the  timer	
	
	ctx.op_num = 0;
	while (ctx.op_num  <  OPERATIONS_CNT) {
		ctx.op_num = ctx.op_num + 1;
		DEBUG_COUT (std::endl << "[Info] Submitting lock request #" << ctx.op_num);

		// ************************************************************************
		//	Client request a lock on the selected item and waits for a response.
		select_item(ctx.lock_request);
		req_cpy.request_item = ctx.lock_request.request_item;		// to keep track of the initial request type, because ctx.lock_request is going to change
		req_cpy.request_type = ctx.lock_request.request_type;		// to keep track of the initial request type, because ctx.lock_request is going to change
		
		clock_gettime(CLOCK_REALTIME, &beforeAcquisition); // Fire the  timer		
		acquire_lock (ctx, ctx.lock_request, ctx.lock_response);
		clock_gettime(CLOCK_REALTIME, &afterAcquisition);	// Fire the  timer
		acquisitionTime = ( afterAcquisition.tv_sec - beforeAcquisition.tv_sec ) * 1E6 + ( afterAcquisition.tv_nsec - beforeAcquisition.tv_nsec )/1E3;
		
		// hold lock for a specified time period (in a specific way).
		hold_lock();
		
		// ************************************************************************
		//	Clients release the acquired lock.
		ctx.lock_request.request_type = LockRequest::RELEASE;
		
		clock_gettime(CLOCK_REALTIME, &beforeRelease); // Fire the  timer		
		release_lock (ctx, ctx.lock_request, ctx.lock_response);
		clock_gettime(CLOCK_REALTIME, &afterRelease);	// Fire the  timer
		releaseTime = ( afterRelease.tv_sec - beforeRelease.tv_sec ) * 1E6 + ( afterRelease.tv_nsec - beforeRelease.tv_nsec )/1E3;
		
		if(req_cpy.request_type == LockRequest::EXCLUSIVE){
			sumExclusiveAcqTime += acquisitionTime;
			sumExclusiveRelTime += releaseTime; 
			exclusiveCount++;
		}
		else if(req_cpy.request_type == LockRequest::SHARED){
			sumSharedAcqTime += acquisitionTime; 
			sumSharedRelime += releaseTime; 
			sharedCount++;
		}
	}
	clock_gettime(CLOCK_REALTIME, &lastRequestTime); // Fire the  timer

	double micro_elapsed_time = ( lastRequestTime.tv_sec - firstRequestTime.tv_sec ) * 1E6 + ( lastRequestTime.tv_nsec - firstRequestTime.tv_nsec )/1E3;
	double lock_per_sec = (double)(OPERATIONS_CNT / (double)(micro_elapsed_time / 1000000));
	
	std::cout << std::endl << "[Stat] Locks (acquire & release) per sec: 	" <<  lock_per_sec << std::endl;
	std::cout << "[STAT] Avg time per Exclusive acquisition (us)	" << sumExclusiveAcqTime / exclusiveCount << std::endl;
	std::cout << "[STAT] Avg time per Exclusive release (us)		" << sumExclusiveRelTime / exclusiveCount << std::endl;
	std::cout << "[STAT] Avg time per Shared acquisition (us)		" << sumSharedAcqTime / sharedCount << std::endl;
	std::cout << "[STAT] Avg time per Shared release (us)			" << sumSharedRelime / sharedCount << std::endl;

	return 0;
}

int SRLockClient::select_item (struct LockRequest &req) {
	int target_idx = 0;
	if (ITEM_CNT <= RAND_MAX){
		target_idx = rand() % ITEM_CNT; // 0 to ITEM_CNT-1
	}
	else {
		int num_fragment = (int) ITEM_CNT / RAND_MAX + 1;
		int target_idx = ((rand() % num_fragment)*RAND_MAX - 1) + (rand() % ITEM_CNT);
	}
	
	
	double pdf = (double) rand()/RAND_MAX; // 50-50 exclusive and shared locks
	req.request_item = target_idx;
	if (pdf >= SHARED_TO_MIX_RATIO){
		req.request_type = LockRequest::EXCLUSIVE;
	}
	else {
		req.request_type = LockRequest::SHARED;
	}
	
	return 0;
}

int SRLockClient::acquire_lock (SRClientContext &ctx, struct LockRequest &req, struct LockResponse &res) {
	if (req.request_type == LockRequest::SHARED || req.request_type == LockRequest::EXCLUSIVE) {
		TEST_NZ (RDMACommon::post_RECEIVE(ctx.qp, ctx.lock_res_mr, (uintptr_t)&res, sizeof(struct LockResponse))); 
		DEBUG_COUT("[Info] receive posted to the queue");
		
		// TEST_NZ (sock_write(ctx.sockfd, (char *)&req, sizeof(struct LockRequest)));
		TEST_NZ (RDMACommon::post_SEND(ctx.qp, ctx.lock_req_mr, (uintptr_t)&req, sizeof(struct LockRequest), true));
		TEST_NZ (RDMACommon::poll_completion(ctx.cq));	// Ack for SEND
			
		DEBUG_COUT("[Sent] LockRequest::Request (" << req.request_type << ") to LM.");
		
		//TEST_NZ (sock_read(ctx.sockfd, (char *)&res, sizeof(struct LockResponse)));
		TEST_NZ (RDMACommon::poll_completion(ctx.cq));	// Receive LockResponse
		
		if (res.response_type == LockResponse::GRANTED)
			DEBUG_COUT("[Recv] " << req.request_type << " LockResponse (result: granted)");
		else {
			DEBUG_CERR("[Error] " << req.request_type << "LockResponse (result: failed)");
		}
				
		return 0;
	}
	return 1;
}

int SRLockClient::release_lock (SRClientContext &ctx, struct LockRequest &req, struct LockResponse &res) {
	if (req.request_type == LockRequest::RELEASE){
		TEST_NZ (RDMACommon::post_RECEIVE(ctx.qp, ctx.lock_res_mr, (uintptr_t)&res, sizeof(struct LockResponse))); 
		DEBUG_COUT("[Info] receive posted to the queue");
		
		//TEST_NZ (sock_write(ctx.sockfd, (char *)&req, sizeof(struct LockRequest)));
		TEST_NZ (RDMACommon::post_SEND(ctx.qp, ctx.lock_req_mr, (uintptr_t)&req, sizeof(struct LockRequest), true));
		TEST_NZ (RDMACommon::poll_completion(ctx.cq));	// Ack for SEND
			
		DEBUG_COUT("[Sent] LockRequest::Request (RELEASE) to LM.");
		
		//TEST_NZ (sock_read(ctx.sockfd, (char *)&res, sizeof(struct LockResponse)));
		TEST_NZ (RDMACommon::poll_completion(ctx.cq));	// Receive LockResponse
		
		if (res.response_type == LockResponse::RELEASED)
			DEBUG_COUT("[Recv] RELEASE LockResponse (result: released)");
		else {
			DEBUG_COUT("[Recv] RELEASE LockResponse (result: failed)");
		}
		return 0;
	}
	return 1;
}

int SRLockClient::hold_lock () { 
	// hold onto lock for some pre-determined time
	sleep_for_microsec(LOCK_OWNERSHIP_MEAN_USEC);
	
	return 0;
}

int SRLockClient::start_client () {	
	SRClientContext ctx;
	char temp_char;
	
	ctx.ib_port = SERVER_IB_PORT;
	
	srand (generate_random_seed());		// initialize random seed
		
	TEST_NZ (establish_tcp_connection(SERVER_ADDR.c_str(), SERVER_TCP_PORT, &(ctx.sockfd)));
	
	DEBUG_COUT("[Comm] Client connected to LM on sock " << ctx.sockfd);

	TEST_NZ (ctx.create_context());
	DEBUG_COUT("[Info] Context Created " << ctx.sockfd);
	
	
	TEST_NZ (RDMACommon::connect_qp (&(ctx.qp), ctx.ib_port, ctx.port_attr.lid, ctx.sockfd));	
	DEBUG_COUT("[Conn] QP connected!");
	
	start_operation(ctx);
	
	// Sync so server will know that client is done mucking with its memory
	DEBUG_COUT("[Info] Client is done, and is ready to destroy its resources!");
	TEST_NZ (sock_sync_data (ctx.sockfd, 1, "W", &temp_char));	/* just send a dummy char back and forth */
	TEST_NZ(ctx.destroy_context());
}

void SRLockClient::usage (const char *argv0) {
	std::cout << "Usage:" << std::endl;
	std::cout << "..." << std::endl;
}

int main (int argc, char *argv[]) {
	if (argc != 1) {
		SRLockClient::usage(argv[0]);
		return 1;
	}
	SRLockClient client;
	client.start_client();
	return 0;
}
