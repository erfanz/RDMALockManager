/*
 *	CombinedServerContext.hpp
 *
 *	Created on: 26.Mar.2015
 *	Author: erfanz
 */

#ifndef COMBINED_SERVER_CONTEXT_H_
#define COMBINED_SERVER_CONTEXT_H_

#include "../util/BaseContext.hpp"
#include "../lock/LockRequest.h"
#include "../lock/LockResponse.h"
#include "../config.hpp"
#include "CombinedMemoryKeys.hpp"
#include "../util/RDMACommon.hpp"
#include <iostream>
#include <atomic>

class CombinedServerContext : public BaseContext {
public:
	std::string	server_address;	
	
	int client_id;						// The client identifier (starts from 1)
	
	// memory handlers
	struct ibv_mr *send_message_mr;
	struct ibv_mr *item_lock_mr;
	struct ibv_mr *outstanding_mr;		// the memory handler for receiving SHARED and EXCLUSIVE lock requests (not RELEASE)			
	struct ibv_mr *lock_res_mr;			// for lock response
	
	// memory buffers
	struct CombinedMemoryKeys send_message_msg;
	struct LockResponse lock_response;
	
	
	//uint64_t* item_lock_array;
	std::atomic<uint64_t> *item_lock_array;
	
	LockRequest  *outstanding_array;
	
	// char *send_data_msg;
	// int recv_data_msg;
	
	
	int register_memory();
	int destroy_context ();
};
#endif // COMBINED_SERVER_CONTEXT_H_