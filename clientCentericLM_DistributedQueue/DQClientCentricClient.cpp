/*
 *	DQClientCentricClient.cpp
 *
 *	Created on: 1.May.2015
 *	Author: erfanz
 */

#include "DQClientCentricClient.hpp"
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

void DQClientCentricClient::usage (const char *argv0) {
	std::cout << "Usage:" << std::endl;
	std::cout << argv0 << " connects to server(s) specified in the config file" << std::endl;
}

/******************** For Read
int BenchmarkClient::start_benchmark(DQClientContext *ctx) {
	int		server_num;
	int		abort_cnt = 0;
	bool	abort_flag;
	double cumulative_latency = 0;
	int signaledPosts = 0;
	struct timespec firstRequestTime, lastRequestTime;				// for calculating TPMS
	struct timespec beforeSending, afterSending;				// for calculating TPMS
	
	struct timespec before_read_ts, after_read_ts, after_fetch_info, after_commit_ts, after_lock, after_decrement, after_unlock;
    
	struct rusage usage;
    struct timeval start_user_usage, start_kernel_usage, end_user_usage, end_kernel_usage;
	char temp_char;
	
	
	TEST_NZ (sock_sync_data (ctx->sockfd, 1, "W", &temp_char));	// just send a dummy char back and forth
	
	
	DEBUG_COUT ("[Info] Benchmark now gets started");
	
	clock_gettime(CLOCK_REALTIME, &firstRequestTime);	// Fire the  timer
    getrusage(RUSAGE_SELF, &usage);
    start_kernel_usage = usage.ru_stime;
    start_user_usage = usage.ru_utime;
	
	int offset;
	uint64_t *remote_address;
	
	int iteration = 0;
	while (iteration < OPERATIONS_CNT) {
		offset = rand() % (SERVER_REGION_SIZE - BUFFER_SIZE);
		remote_address = (uint64_t *)(offset + ((uint64_t)ctx->peer_data_mr.addr));
		
		if (iteration % 1000 == 0) {
			//clock_gettime(CLOCK_REALTIME, &beforeSending);	// Fire the  timer
			
			TEST_NZ (RDMACommon::post_RDMA_READ_WRT(IBV_WR_RDMA_READ,
			ctx->qp,
			ctx->recv_data_mr,
			(uint64_t)ctx->recv_data_msg,
			&(ctx->peer_data_mr),
			(uint64_t)remote_address,
			(uint32_t)BUFFER_SIZE,
			true));
			
			TEST_NZ (RDMACommon::poll_completion(ctx->cq));
			//TEST_NZ (RDMACommon::event_based_poll_completion(ctx->comp_channel, ctx->cq));
			
			//clock_gettime(CLOCK_REALTIME, &afterSending);	// Fire the  timer
			
			//cumulative_latency =+ ( ( afterSending.tv_sec - beforeSending.tv_sec ) * 1E9 + ( afterSending.tv_nsec - beforeSending.tv_nsec ) );
			//signaledPosts++;
		}
		else {
			TEST_NZ (RDMACommon::post_RDMA_READ_WRT(IBV_WR_RDMA_READ,
			ctx->qp,
			ctx->recv_data_mr,
			(uint64_t)ctx->recv_data_msg,
			&(ctx->peer_data_mr),
			(uint64_t)remote_address,
			(uint32_t)BUFFER_SIZE,
			false));
		}
	
		//DEBUG_COUT("[RDMA] Response sent");
		//DEBUG_COUT("[Info] Buffer value: " << ctx->local_buffer);
		iteration++;
	}

    getrusage(RUSAGE_SELF, &usage);
    end_user_usage = usage.ru_utime;
    end_kernel_usage = usage.ru_stime;
	
	clock_gettime(CLOCK_REALTIME, &lastRequestTime);	// Fire the  timer
	double user_cpu_microtime = ( end_user_usage.tv_sec - start_user_usage.tv_sec ) * 1E6 + ( end_user_usage.tv_usec - start_user_usage.tv_usec );
	double kernel_cpu_microtime = ( end_kernel_usage.tv_sec - start_kernel_usage.tv_sec ) * 1E6 + ( end_kernel_usage.tv_usec - start_kernel_usage.tv_usec );
	
	double micro_elapsed_time = ( ( lastRequestTime.tv_sec - firstRequestTime.tv_sec ) * 1E9 + ( lastRequestTime.tv_nsec - firstRequestTime.tv_nsec ) ) / 1000;
	
	//double latency_in_micro = (double)(micro_elapsed_time / OPERATIONS_CNT);
	double latency_in_micro = (double)(cumulative_latency / signaledPosts) / 1000;
	
	double mega_byte_per_sec = ((BUFFER_SIZE * OPERATIONS_CNT / 1E6 ) / (micro_elapsed_time / 1E6) );
	double operations_per_sec = OPERATIONS_CNT / (micro_elapsed_time / 1E6);
	double cpu_utilization = (user_cpu_microtime + kernel_cpu_microtime) / micro_elapsed_time;
	
	std::cout << "[Stat] Avg latency(u sec):   	" << latency_in_micro << std::endl; 
	std::cout << "[Stat] MegaByte per Sec:   	" << mega_byte_per_sec <<  std::endl;
	std::cout << "[Stat] Operations per Sec:   	" << operations_per_sec <<  std::endl;
	std::cout << "[Stat] CPU utilization:    	" << cpu_utilization << std::endl;
	std::cout << "[Stat] USER CPU utilization:    	" << user_cpu_microtime / micro_elapsed_time << std::endl;
	std::cout << "[Stat] KERNEL CPU utilization:    	" << kernel_cpu_microtime / micro_elapsed_time << std::endl;
	
	std::cout  << latency_in_micro << '\t' << mega_byte_per_sec << '\t' << operations_per_sec << '\t' << cpu_utilization << std::endl;
	
	return 0;
}
*/


bool DQClientCentricClient::re_inquire_shared_lock(DQClientContext &ctx, int lock_id) {
	uint32_t exclusive_part;
	uint64_t lock_offset = (uint64_t)lock_id * sizeof(uint64_t);
	uint64_t *lock_lookup_address	= (uint64_t *)(lock_offset + ((uint64_t)ctx.peer_data_mr.addr));
	
	TEST_NZ (RDMACommon::post_RDMA_READ_WRT(IBV_WR_RDMA_READ,
	ctx.qp,
	ctx.lock_result_mr,
	(uint64_t)&ctx.lock_result,
	&(ctx.peer_data_mr),
	(uint64_t)lock_lookup_address,
	(uint32_t)sizeof(uint64_t),
	true));
	TEST_NZ (RDMACommon::poll_completion (ctx.cq));		// completion for WRITE
	
	exclusive_part = (uint32_t) (ctx.lock_result >> 32);
		
	// check if the exclusive part is zero
	if (exclusive_part == 0) {
		// The lock is not exclusively owned, and therefore the client now holds the shared lock.
		DEBUG_COUT("[READ] Succeeded: Shared lock re-inquiry for lock " <<  lock_id);
		return true;
	}
	else {
		// The values don't match up. Therefore, the lock is currently not free.
		DEBUG_COUT("[READ] Failed: Shared lock re-inquiry for lock " <<  lock_id);
		return false;
	}
}

bool DQClientCentricClient::request_exclusive_lock(DQClientContext &ctx, int lock_id) {
	uint64_t expected_value = 0ULL;
	uint64_t swap_value = (uint64_t) ctx.client_identifier << 32 | 0;
	
	uint64_t lock_offset = (uint64_t)lock_id * sizeof(uint64_t);
	uint64_t *lock_lookup_address	= (uint64_t *)(lock_offset + ((uint64_t)ctx.peer_data_mr.addr));
	
	TEST_NZ (RDMACommon::post_RDMA_CMP_SWAP(ctx.qp,
	ctx.lock_result_mr,
	(uint64_t)&ctx.lock_result,
	&(ctx.peer_data_mr),
	(uint64_t)lock_lookup_address,
	(uint32_t)sizeof(uint64_t),
	expected_value,
	swap_value));
	
	TEST_NZ (RDMACommon::poll_completion(ctx.cq));
	
	DEBUG_COUT("[CMSW] Compare and Swap the the server's counter");
	
	// Byte swapping, since all values read by atomic operations are in reverse bit order
	ctx.lock_result = hton64(ctx.lock_result);
	
	DEBUG_COUT("[Info] Previous value of lock: (" << (ctx.lock_result >> 32) << "|" << (ctx.lock_result << 32 >> 32)
		<< ")\tExpected value: (" << (expected_value >> 32) << "|" << (expected_value << 32 >> 32)
			<< ")\tSwap value: (" << (swap_value >> 32) << "|" << (swap_value << 32 >> 32) << ")");
	
	// check if the lock result is equal to the expected value
	if (ctx.lock_result == expected_value) {
		// The lock was free and the lock acquisition was successful.
		// The client now holds the lock.
		DEBUG_COUT("[Info] Succeeded: Exclusive lock acquisition for lock " <<  lock_id);
		return true;
		
	}
	else {
		// The values don't match up. Therefore, the lock is currently not free.
		DEBUG_COUT("[Info] Failed: Exclusive lock acquisition for lock " <<  lock_id);
		return false;
	}
}

bool DQClientCentricClient::request_shared_lock(DQClientContext &ctx, int lock_id) {
	uint64_t exclusive_part;
	
	uint64_t lock_offset = (uint64_t)lock_id * sizeof(uint64_t);
	uint64_t *lock_lookup_address	= (uint64_t *)(lock_offset + ((uint64_t)ctx.peer_data_mr.addr));
	
	TEST_NZ (RDMACommon::post_RDMA_FETCH_ADD(ctx.qp,
	ctx.lock_result_mr,
	(uint64_t)&ctx.lock_result,
	&(ctx.peer_data_mr),
	(uint64_t)lock_lookup_address,
	1ULL,
	(uint32_t)sizeof(uint64_t)));
	TEST_NZ (RDMACommon::poll_completion(ctx.cq));
	
	exclusive_part = (uint32_t) (ctx.lock_result >> 32);
		
	// check if the exclusive part is zero
	if (exclusive_part == 0) {
		// The lock is not exclusively owned, and therefore the client now holds the shared lock.
		DEBUG_COUT("[FTCH] Succeeded: Shared lock acquisition for lock " <<  lock_id);
		return true;
	}
	else {
		// The values don't match up. Therefore, the lock is currently not free.
		DEBUG_COUT("[FTCH] Failed: Shared lock acquisition for lock " <<  lock_id);
		return false;
	}
	return 0;
}

int DQClientCentricClient::release_exclusive_lock(DQClientContext &ctx, int lock_id) {
	uint64_t c_id = ctx.client_identifier;
	uint64_t decrement =  (uint64_t) -(c_id << 32);
	
	uint64_t lock_offset = (uint64_t)lock_id * sizeof(uint64_t);
	uint64_t *lock_lookup_address	= (uint64_t *)(lock_offset + ((uint64_t)ctx.peer_data_mr.addr));
	
	TEST_NZ (RDMACommon::post_RDMA_FETCH_ADD(ctx.qp,
	ctx.lock_result_mr,
	(uint64_t)&ctx.lock_result,
	&(ctx.peer_data_mr),
	(uint64_t)lock_lookup_address,
	decrement,
	(uint32_t)sizeof(uint64_t)));
	TEST_NZ (RDMACommon::poll_completion(ctx.cq));
	
	DEBUG_COUT("[FTCH] Succeeded Exclusive lock released");
	return 0;
}

int DQClientCentricClient::release_shared_lock(DQClientContext &ctx, int lock_id) {
	uint64_t decrement =  -1;
	uint64_t lock_offset = (uint64_t)lock_id * sizeof(uint64_t);
	uint64_t *lock_lookup_address	= (uint64_t *)(lock_offset + ((uint64_t)ctx.peer_data_mr.addr));
	
	TEST_NZ (RDMACommon::post_RDMA_FETCH_ADD(ctx.qp,
	ctx.lock_result_mr,
	(uint64_t)&ctx.lock_result,
	&(ctx.peer_data_mr),
	(uint64_t)lock_lookup_address,
	decrement,
	(uint32_t)sizeof(uint64_t)));
	TEST_NZ (RDMACommon::poll_completion(ctx.cq));
	
	DEBUG_COUT("[FTCH] Succeeded: Shared lock released");
	return 0;
}


int DQClientCentricClient::start_benchmark(DQClientContext &ctx) {
	double cumulative_latency = 0, elapsed_shared_time = 0, elapsed_exclusive_time = 0;
	int signaledPosts = 0;
	struct timespec firstRequestTime, lastRequestTime;				// for calculating TPMS
	struct timespec beforeSending, afterSending;				// for calculating TPMS

	struct timespec before_read_ts, after_read_ts, after_fetch_info, after_commit_ts, after_lock, after_decrement, after_unlock;

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
		lock_id = rand() % (ITEM_CNT);
		
		abort_flag = false;
		attemp_cnt = 1;
		
		if (rand() % 2 == 0){
			clock_gettime(CLOCK_REALTIME, &firstRequestTime);	// Fire the  timer
			
			// Go for shared lock
			total_shared_request++;
			
			// first, client registers its request in the server's memory
			if (request_shared_lock(ctx, lock_id) == true) {
				// the shared lock acquisition was successful
				;
			}
			else {
				attemp_cnt++;
				while (re_inquire_shared_lock(ctx, lock_id) == false)
				{
					if (attemp_cnt >= MAXIMUM_ATTEMPTS) {
						DEBUG_COUT("[Info] Client gives up at requesting for lock " << lock_id);
						abort_flag = true;
						aborted_shared++;
						break;
					}
					// The client backs off for a pre-defined amount of time.
					DEBUG_COUT("[Info] Client backs off for " << BACK_OFF_MICRO_SEC << " micro seconds and try again");
					sleep_for_microsec(BACK_OFF_MICRO_SEC);
					attemp_cnt++;
				}
			}
			total_shared_attemps += attemp_cnt;
			
			if (abort_flag == false){
				// hold onto lock for some pre-determined time
				sleep_for_microsec(LOCK_OWNERSHIP_MEAN_USEC);
			}
			
			// de-register from the server
			TEST_NZ(release_shared_lock(ctx, lock_id));
			DEBUG_COUT("");
			clock_gettime(CLOCK_REALTIME, &lastRequestTime);	// Fire the  timer
			elapsed_shared_time += ( ( lastRequestTime.tv_sec - firstRequestTime.tv_sec ) * 1E9 + ( lastRequestTime.tv_nsec - firstRequestTime.tv_nsec ) ) / 1000;
				
		}
		else {
			// Go for exclusive lock 
			clock_gettime(CLOCK_REALTIME, &firstRequestTime);	// Fire the  timer
			
			total_exclusive_request++;
			
			while (request_exclusive_lock(ctx, lock_id) == false)
			{
				if (attemp_cnt >= MAXIMUM_ATTEMPTS) {
					DEBUG_COUT("[Info] Client gives up at requesting for lock " << lock_id);
					abort_flag = true;
					aborted_exclusive++;
					break;
				}
				// The client backs off for a pre-defined amount of time.
				DEBUG_COUT("[Info] Client backs off for " << BACK_OFF_MICRO_SEC << " micro seconds and try again");
				sleep_for_microsec(BACK_OFF_MICRO_SEC);
				attemp_cnt++;
			}
			total_exclusive_attemps += attemp_cnt;
		
			// release the lock, if holds it
			if (abort_flag == false){
				// hold onto lock for some pre-determined time, and then release it
				sleep_for_microsec(LOCK_OWNERSHIP_MEAN_USEC);
				
				release_exclusive_lock(ctx, lock_id);
			}
			DEBUG_COUT("");
			clock_gettime(CLOCK_REALTIME, &lastRequestTime);	// Fire the  timer
			
			elapsed_exclusive_time += ( ( lastRequestTime.tv_sec - firstRequestTime.tv_sec ) * 1E9 + ( lastRequestTime.tv_nsec - firstRequestTime.tv_nsec ) ) / 1000;
		}
		iteration++;
	}
	
	std::cout << "[STAT] Avg request per Exclusive op	" << (double)total_exclusive_attemps / total_exclusive_request << std::endl;
	std::cout << "[STAT] Avg request per Shared    op	" << (double)total_shared_attemps / total_shared_request << std::endl;
	std::cout << "[STAT] Avg time per Exclusive op (us)	" << (double)elapsed_exclusive_time / total_exclusive_request << std::endl;
	std::cout << "[STAT] Avg time per Shared    op (us)	" << (double)elapsed_shared_time / total_shared_request << std::endl;
	std::cout << "[STAT] Abort rate for Exclusive op	" << (double)aborted_exclusive / total_exclusive_request << std::endl;
	std::cout << "[STAT] Abort rate for Shared    op	" << (double)aborted_shared / total_shared_request << std::endl;
		
	/*

    getrusage(RUSAGE_SELF, &usage);
    end_user_usage = usage.ru_utime;
    end_kernel_usage = usage.ru_stime;

	clock_gettime(CLOCK_REALTIME, &lastRequestTime);	// Fire the  timer
	double user_cpu_microtime = ( end_user_usage.tv_sec - start_user_usage.tv_sec ) * 1E6 + ( end_user_usage.tv_usec - start_user_usage.tv_usec );
	double kernel_cpu_microtime = ( end_kernel_usage.tv_sec - start_kernel_usage.tv_sec ) * 1E6 + ( end_kernel_usage.tv_usec - start_kernel_usage.tv_usec );

	double micro_elapsed_time = ( ( lastRequestTime.tv_sec - firstRequestTime.tv_sec ) * 1E9 + ( lastRequestTime.tv_nsec - firstRequestTime.tv_nsec ) ) / 1000;

	//double latency_in_micro = (double)(micro_elapsed_time / OPERATIONS_CNT);
	double latency_in_micro = (double)(cumulative_latency / signaledPosts) / 1000;

	double mega_byte_per_sec = ((BUFFER_SIZE * OPERATIONS_CNT / 1E6 ) / (micro_elapsed_time / 1E6) );
	double operations_per_sec = OPERATIONS_CNT / (micro_elapsed_time / 1E6);
	double cpu_utilization = (user_cpu_microtime + kernel_cpu_microtime) / micro_elapsed_time;

	std::cout << "[Stat] Avg latency(u sec):   	" << latency_in_micro << std::endl;
	std::cout << "[Stat] MegaByte per Sec:   	" << mega_byte_per_sec <<  std::endl;
	std::cout << "[Stat] Operations per Sec:   	" << operations_per_sec <<  std::endl;
	std::cout << "[Stat] CPU utilization:    	" << cpu_utilization << std::endl;
	std::cout << "[Stat] USER CPU utilization:    	" << user_cpu_microtime / micro_elapsed_time << std::endl;
	std::cout << "[Stat] KERNEL CPU utilization:    	" << kernel_cpu_microtime / micro_elapsed_time << std::endl;

	std::cout  << latency_in_micro << '\t' << mega_byte_per_sec << '\t' << operations_per_sec << '\t' << cpu_utilization << std::endl;
	*/
	

		
		
	return 0;
}


// int DQClientCentricClient::connect_to_other_clients(DQClientContext &ctx){
// 	for (int c_id = 0; c_id < ctx.client_identifier; c_id++) {
// 		// active in making connections to these clients.
// 		if (establish_tcp_connection(ctx.server_address,  SERVER_TCP_PORT, &ctx.clients_sock[c_id].sockfd)) != 0) {
// 			DEBUG_COUT("[Conn] [Client " << ctx.client_identifier <<"] Failed to connect to client " << c_id << "
// 				(" << CLIENTS_ADDR[c_id] << ":" << CLIENTS_TCP_PORT[c_id] << "). Will retry in some moments.");
//
// 			usleep(50);	// sleep for 50 micro seconds
//
//
//
// 		}
// 		DEBUG_COUT("[Conn] Connection established to server");
//
// 	}
// 	for (int c_id = ctx.client_identifier + 1; c_id++){
// 		// passive in making connections to these clients
//
//
// 	}
// }


int DQClientCentricClient::start_client () {	
	DQClientContext ctx;
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
	RDMACommon::post_RECEIVE(ctx.qp, ctx.recv_memory_mr, (uintptr_t)&ctx.recv_memory_msg, sizeof(struct MemoryKeys));
	
	TEST_NZ (RDMACommon::connect_qp (&(ctx.qp), ctx.ib_port, ctx.port_attr.lid, ctx.sockfd));

	TEST_NZ(RDMACommon::poll_completion(ctx.cq));
	DEBUG_COUT("[Recv] buffers info from server");
	
	// after receiving the message from the server, let's store its addresses in the context
	memcpy(&ctx.peer_data_mr,	&ctx.recv_memory_msg.peer_mr,	sizeof(struct ibv_mr));
	ctx.client_identifier = ctx.recv_memory_msg.client_id;
	ctx.clients_addr_vector =  split(ctx.recv_memory_msg.clients_addr, '|');
	ctx.clients_tcp_port_vector =  split(ctx.recv_memory_msg.clients_tcp_port, '|');

	
	// for(std::vector<std::string>::const_iterator i = ctx.clients_addr_vector.begin(); i != ctx.clients_addr_vector.end(); ++i)
	//     std::cout << *i << ' ';
	
	// Connect to other clients
	//connect_to_other_clients(ctx);
	
	DEBUG_COUT("[Conn] QPed to server, and was assigned ID = " << ctx.client_identifier);
	
	DEBUG_COUT ("[Info] Successfully connected to server");
	
	//start_benchmark(ctx);
	
	DEBUG_COUT("[Info] Client is done, and is ready to destroy its resources!");
	TEST_NZ (sock_sync_data (ctx.sockfd, 1, "W", &temp_char));
	DEBUG_COUT("[Conn] Notified server it's done");
	TEST_NZ ( ctx.destroy_context());
}

int main (int argc, char *argv[]) {
	if (argc != 1) {
		DQClientCentricClient::usage(argv[0]);
		return 1;
	}
	DQClientCentricClient client;
	client.start_client();
	return 0;
}