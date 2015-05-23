/*
 *	CombinedClient.cpp
 *
 *	Created on: 1.May.2015
 *	Author: erfanz
 */

#include "CombinedClient.hpp"
#include "../util/utils.hpp"
#include "../config.hpp"

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
#include <netdb.h>
#include <iostream>
#include <time.h>	// for struct timespec
#include <sys/resource.h>	// getrusage()
#include <math.h>	// for sin() which we need for dummy calculation

void CombinedClient::usage (const char *argv0) {
	std::cout << "Usage:" << std::endl;
	std::cout << argv0 << " connects to server(s) specified in the config file" << std::endl;
}

int CombinedClient::select_item (struct LockRequest &req) {
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
	if (true || pdf >= SHARED_TO_MIX_RATIO)
		req.request_type = LockRequest::EXCLUSIVE;
	else 
		req.request_type = LockRequest::SHARED;
	return 0;
}

int CombinedClient::hold_lock () { 
	// hold onto lock for some pre-determined time
	sleep_for_microsec(LOCK_OWNERSHIP_MEAN_USEC);	
	return 0;
}

bool CombinedClient::try_acquire_shared_lock(CombinedClientContext &ctx) {
	int lock_id = ctx.lock_request.request_item;
	uint64_t exclusive_part;
	uint64_t lock_offset = (uint64_t)lock_id * sizeof(uint64_t);
	uint64_t *lock_lookup_address	= (uint64_t *)(lock_offset + ((uint64_t)ctx.peer_item_lock_mr.addr));

	TEST_NZ (RDMACommon::post_RDMA_FETCH_ADD(ctx.qp,
	ctx.item_lock_mr,
	(uint64_t)&ctx.item_lock,
	&(ctx.peer_item_lock_mr),
	(uint64_t)lock_lookup_address,
	1ULL,
	(uint32_t)sizeof(uint64_t)));
	TEST_NZ (RDMACommon::poll_completion(ctx.cq));

	exclusive_part = (uint32_t) (ctx.item_lock >> 32);

	// check if the exclusive part is zero
	if (exclusive_part == 0) {
		// The lock is not exclusively owned, and therefore the client now holds the shared lock.
		DEBUG_COUT("[FTCH] Succeeded: Shared lock acquired for lock " <<  lock_id);
		return true;
	}
	else {
		// The values don't match up. Therefore, the lock is currently not free.
		DEBUG_COUT("[FTCH] Failed: Shared lock not acquired for lock " <<  lock_id);
		return false;
	}
	return 0;
}

bool CombinedClient::try_acquire_exclusive_lock(CombinedClientContext &ctx) {	
	int lock_id = ctx.lock_request.request_item;
	uint64_t expected_value = 0ULL;
	uint64_t swap_value = (uint64_t) ctx.client_identifier << 32 | 0;
		
	uint64_t lock_offset = (uint64_t)lock_id * sizeof(uint64_t);
	uint64_t *lock_lookup_address	= (uint64_t *)(lock_offset + ((uint64_t)ctx.peer_item_lock_mr.addr));
	
	lock_id = 0;
		
	TEST_NZ (RDMACommon::post_RDMA_CMP_SWAP(ctx.qp,
	ctx.item_lock_mr,
	(uint64_t)&ctx.item_lock,
	&(ctx.peer_item_lock_mr),
	(uint64_t)lock_lookup_address,
	(uint32_t)sizeof(uint64_t),
	expected_value,
	swap_value));
	
	TEST_NZ (RDMACommon::poll_completion(ctx.cq));
	
	DEBUG_COUT("[CMSW] Compare and Swap the the server's memory for item " << lock_id);
	
	// Byte swapping, since all values read by atomic operations are in reverse bit order
	ctx.item_lock = hton64(ctx.item_lock);
	
	DEBUG_COUT("[Info] Previous value of lock: (" << (ctx.item_lock >> 32) << "|" << (ctx.item_lock << 32 >> 32)
		<< ")\tExpected value: (" << (expected_value >> 32) << "|" << (expected_value << 32 >> 32)
			<< ")\tSwap value: (" << (swap_value >> 32) << "|" << (swap_value << 32 >> 32) << ")");
	
	// check if the lock result is equal to the expected value
	if (ctx.item_lock == expected_value) {
		// The lock was free and the lock acquisition was successful.
		// The client can now hold the lock.
		DEBUG_COUT("[Info] Succeeded: Exclusive lock acquired for lock " <<  lock_id);
		return true;
		
	}
	else {
		// The values don't match up. Therefore, the lock is currently not free.
		DEBUG_COUT("[Info] Failed: Exclusive lock acquired for lock " <<  lock_id);
		return false;
	}
}

int CombinedClient::acquire_shared_lock(CombinedClientContext &ctx) {
	DEBUG_COUT("[Info] First, it tries the DIRECT way to acquire the SHARED lock for item " << ctx.lock_request.request_item);
	if (try_acquire_shared_lock(ctx) == false) {
		// the shared lock DIRECT acquisition failed
		DEBUG_COUT("[Info] SHARED DIRECT lock acquisition (SHARED) failed on item " << ctx.lock_request.request_item);
		TEST_NZ (register_request_as_outstanding(ctx));
		
		// lock is acquired at this point
		DEBUG_COUT("[RECV] Lock response RELEASE  (result: released) for item " << ctx.lock_request.request_item);
	}
	return 0;
}

int CombinedClient::acquire_exclusive_lock(CombinedClientContext &ctx) {
	DEBUG_COUT("[Info] First, it tries the DIRECT way to acquire the EXCLUSIVE lock for item " << ctx.lock_request.request_item);
	if (try_acquire_exclusive_lock(ctx) == false) {
		// the exclusive DIRECT lock acquisition failed
		DEBUG_COUT("[Info] the DIRECT lock acquisition (EXCLUSIVE) failed on item " << ctx.lock_request.request_item);
		TEST_NZ (register_request_as_outstanding(ctx));
		
		// lock is acquired at this point
		DEBUG_COUT("[RECV] Lock response RELEASE  (result: released) for item " << ctx.lock_request.request_item);
	}
	return 0;
}


int CombinedClient::register_request_as_outstanding(CombinedClientContext &ctx){
	// RECEIVE must be posted upfront
	TEST_NZ (RDMACommon::post_RECEIVE(ctx.qp, ctx.lock_res_mr, (uintptr_t)&ctx.lock_response, sizeof(struct LockResponse))); 
	DEBUG_COUT("[Info] RECEIVE posted to the queue");
	
	uint64_t outstanding_offset = (uint64_t)(ctx.client_identifier - 1) * sizeof(struct LockRequest);
	uint64_t *outstanding_lookup_address	= (uint64_t *)(outstanding_offset + ((uint64_t)ctx.peer_outstanding_mr.addr));

	// The client now registers its request in server's outstanding array
	TEST_NZ (RDMACommon::post_RDMA_READ_WRT(IBV_WR_RDMA_WRITE,
	ctx.qp,
	ctx.lock_req_mr,
	(uint64_t)&ctx.lock_request,
	&(ctx.peer_outstanding_mr),
	(uint64_t)outstanding_lookup_address,
	(uint32_t)sizeof(struct LockRequest),
	true));
	TEST_NZ (RDMACommon::poll_completion (ctx.cq));		// completion for WRITE

	DEBUG_COUT("[WRIT] Request (item, type): (" << ctx.lock_request.request_item << "," << ctx.lock_request.request_type << ") to LM.");
	
	
	// and now waits for the server to grant the lock
	DEBUG_COUT("[INFO] Client is now waiting for the server to grant it the lock");
	
	TEST_NZ (RDMACommon::poll_completion (ctx.cq));		// RECEIVE
	
	if (ctx.lock_response.response_type == LockResponse::GRANTED) { 
		DEBUG_COUT("[RECV] GRANT from the server");
		return 0;
	}
	
	else
		// ERROR, the received resposne must be GRANT
		return -1;
}

int CombinedClient::release_lock (CombinedClientContext &ctx) {
	
	// First, change the request type
	if (ctx.lock_request.request_type == LockRequest::SHARED)
		ctx.lock_request.request_type = LockRequest::RELEASE_SHARED;
	else 
		ctx.lock_request.request_type = LockRequest::RELEASE_EXCLUSIVE;
	
	TEST_NZ (RDMACommon::post_RECEIVE(ctx.qp, ctx.lock_res_mr, (uintptr_t)&ctx.lock_response, sizeof(struct LockResponse))); 
	DEBUG_COUT("[Info] RECEIVE posted to the queue");
	
	
	uint64_t outstanding_offset = (uint64_t)(ctx.client_identifier - 1) * sizeof(struct LockRequest);
	uint64_t *outstanding_lookup_address	= (uint64_t *)(outstanding_offset + ((uint64_t)ctx.peer_outstanding_mr.addr));
	
	TEST_NZ (RDMACommon::post_RDMA_READ_WRT(IBV_WR_RDMA_WRITE,
	ctx.qp,
	ctx.lock_req_mr,
	(uint64_t)&ctx.lock_request,
	&(ctx.peer_outstanding_mr),
	(uint64_t)outstanding_lookup_address,
	(uint32_t)sizeof(struct LockRequest),
	true));
	TEST_NZ (RDMACommon::poll_completion (ctx.cq));		// completion for WRITE
	
	DEBUG_COUT("[WRIT] Request RELEASE to LM for item " << ctx.lock_request.request_item);
	
	TEST_NZ (RDMACommon::poll_completion(ctx.cq));	// Receive LockResponse
	
	if (ctx.lock_response.response_type == LockResponse::RELEASED)
		return 0;
	else {
		DEBUG_COUT("[RECV] Lock response RELEASE  (result: failed) for item " << ctx.lock_request.request_item);
		return -1;
	}
}

int CombinedClient::start_benchmark(CombinedClientContext &ctx) {
	double cumulative_latency = 0, elapsed_shared_time = 0, elapsed_exclusive_time = 0;
	
	struct timespec firstRequestTime, lastRequestTime, beforeAcquisition, afterAcquisition, beforeRelease, afterRelease;
	double sumExclusiveAcqTime= 0.0, sumSharedAcqTime= 0.0, sumExclusiveRelTime= 0.0, sumSharedRelime= 0.0;
	double acquisitionTime, releaseTime;
	int exclusiveCount= 0, sharedCount= 0;

	struct rusage usage;
    struct timeval start_user_usage, start_kernel_usage, end_user_usage, end_kernel_usage;
	char temp_char;
	
	int offset;
	uint64_t *remote_address;

	int lock_id;
	bool result, abort_flag;
	int attemp_cnt, total_shared_attemps = 0, total_exclusive_attemps = 0;
	int total_shared_request = 0, total_exclusive_request = 0;
	int aborted_shared = 0, aborted_exclusive = 0;
	int iteration = 0;
	

	DEBUG_COUT ("[Info] Benchmark now gets started");

	while (iteration < OPERATIONS_CNT) {
		// ************************************************************************
		//	Client request a lock on the selected item and waits for a response.
		select_item(ctx.lock_request);
		
		if (ctx.lock_request.request_type == LockRequest::SHARED){
			total_shared_request++;
			
			DEBUG_COUT ("[Info] Start the operation for SHARED lock for item " << ctx.lock_request.request_item);
			TEST_NZ (acquire_shared_lock(ctx));
		}
		else {
			// ASSERT ctx.lock_request.request_type == LockRequest::EXCLUSIVE
			
			DEBUG_COUT ("[Info] Start the operation for EXCLUSIVE lock for item " << ctx.lock_request.request_item);
			total_exclusive_request++;
			TEST_NZ (acquire_exclusive_lock(ctx));
		}
		
		// hold lock for a specified time period (in a specific way).
		hold_lock();
		
		// submit the RELEASE request 
		DEBUG_COUT ("[Info] Start the operation for releasing lock for item " << ctx.lock_request.request_item);
		TEST_NZ(release_lock(ctx));
		
		iteration++;
	}
	
	// std::cout << "[STAT] Avg request per Exclusive op	" << (double)total_exclusive_attemps / total_exclusive_request << std::endl;
	// std::cout << "[STAT] Avg request per Shared    op	" << (double)total_shared_attemps / total_shared_request << std::endl;
	// std::cout << "[STAT] Avg time per Exclusive op (us)	" << (double)elapsed_exclusive_time / total_exclusive_request << std::endl;
	// std::cout << "[STAT] Avg time per Shared    op (us)	" << (double)elapsed_shared_time / total_shared_request << std::endl;
	// std::cout << "[STAT] Abort rate for Exclusive op	" << (double)aborted_exclusive / total_exclusive_request << std::endl;
	// std::cout << "[STAT] Abort rate for Shared    op	" << (double)aborted_shared / total_shared_request << std::endl;
	
	return 0;
}

int CombinedClient::start_client () {	
	CombinedClientContext ctx;
	char temp_char;
	
	srand (generate_random_seed());		// initialize random seed

	// Connect to server
    ctx.server_address = "";
	ctx.server_address += SERVER_ADDR;
	ctx.ib_port		  = SERVER_IB_PORT;
	TEST_NZ (establish_tcp_connection(ctx.server_address, SERVER_TCP_PORT, &ctx.sockfd));
	DEBUG_COUT("[Conn] Connection established to server");

	TEST_NZ (ctx.create_context());
	
	// before connecting the queue pairs, we post the RECEIVE job to be ready for the server's message containing its memory locations
	RDMACommon::post_RECEIVE(ctx.qp, ctx.recv_memory_mr, (uintptr_t)&ctx.recv_memory_msg, sizeof(struct CombinedMemoryKeys));
	
	TEST_NZ (RDMACommon::connect_qp (&(ctx.qp), ctx.ib_port, ctx.port_attr.lid, ctx.sockfd));

	TEST_NZ(RDMACommon::poll_completion(ctx.cq));
	DEBUG_COUT("[Recv] buffers info from server");
	
	// after receiving the message from the server, let's store its addresses in the context
	memcpy(&ctx.peer_item_lock_mr,		&ctx.recv_memory_msg.peer_item_lock_mr,		sizeof(struct ibv_mr));
	memcpy(&ctx.peer_outstanding_mr,	&ctx.recv_memory_msg.peer_outstanding_mr,	sizeof(struct ibv_mr));	
	ctx.client_identifier = ctx.recv_memory_msg.client_id;
	
	DEBUG_COUT("[Conn] QPed to server, and was assigned ID = " << ctx.client_identifier);
	
	DEBUG_COUT ("[Info] Successfully connected to server");
	
	start_benchmark(ctx);
	
	DEBUG_COUT("[Info] Client is done, and is ready to destroy its resources!");
	TEST_NZ (sock_sync_data (ctx.sockfd, 1, "W", &temp_char));
	DEBUG_COUT("[Conn] Notified server it's done");
	TEST_NZ ( ctx.destroy_context());
}

int main (int argc, char *argv[]) {
	if (argc != 1) {
		CombinedClient::usage(argv[0]);
		return 1;
	}
	CombinedClient client;
	client.start_client();
	return 0;
}