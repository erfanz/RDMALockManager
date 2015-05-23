/*
 *	BClientContext.hpp
 *
 *	Created on: 26.Mar.2015
 *	Author: erfanz
 */

#ifndef B_CLIENT_CONTEXT_H_
#define B_CLIENT_CONTEXT_H_

#include "../util/BaseContext.hpp"
#include "../util/RDMACommon.hpp"
#include "BMemoryKeys.hpp"
#include <iostream>

class BClientContext : public BaseContext{
public:
	std::string	server_address;
	
	uint32_t client_identifier;		// this will be provided by the server
	
	// local memory handler
	struct ibv_mr *recv_memory_mr;			
	struct ibv_mr *lock_result_mr;	
	
	// memory bufferes
	struct BMemoryKeys	recv_memory_msg;
	uint64_t lock_result;
	
	// remote memory handlers
	struct ibv_mr peer_data_mr;
	
	
	int register_memory();
	int destroy_context ();
};
#endif // CLIENT_CONTEXT_H_