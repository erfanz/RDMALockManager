/*
 *	CombinedClientContext.hpp
 *
 *	Created on: 26.Mar.2015
 *	Author: erfanz
 */

#ifndef COMBINED_CLIENT_CONTEXT_H_
#define COMBINED_CLIENT_CONTEXT_H_

#include "../lock/LockRequest.h"
#include "../lock/LockResponse.h"
#include "../util/BaseContext.hpp"
#include "../util/RDMACommon.hpp"
#include "CombinedMemoryKeys.hpp"
#include <iostream>

class CombinedClientContext : public BaseContext{
public:
	std::string	server_address;
	
	uint32_t client_identifier;		// this will be provided by the server
	
	// local memory handler
	struct ibv_mr *recv_memory_mr;			// For message		
	struct ibv_mr *item_lock_mr;			// For the response of the atomic operations for head of the tail 
	struct ibv_mr *lock_req_mr;				// For submitting into the outstanding array
	struct ibv_mr *lock_res_mr;				// For receiving GRANT from server
	
	// memory bufferes
	struct CombinedMemoryKeys	recv_memory_msg;
	uint64_t item_lock;
	LockRequest lock_request;
	LockResponse lock_response;
	
	
	// remote memory handlers
	struct ibv_mr peer_item_lock_mr;
	struct ibv_mr peer_outstanding_mr;		// the remote memory handler for registering SHARED and EXCLUSIVE lock requests (not RELEASE)
	
	
	
	
	int register_memory();
	int destroy_context ();
};
#endif // COMBINED_CLIENT_CONTEXT_H_