/*
 *	BServerContext.hpp
 *
 *	Created on: 26.Mar.2015
 *	Author: erfanz
 */

#ifndef B_SERVER_CONTEXT_H_
#define B_SERVER_CONTEXT_H_

#include "../util/BaseContext.hpp"
#include "BMemoryKeys.hpp"
#include "../util/RDMACommon.hpp"
#include <iostream>

class BServerContext : public BaseContext {
public:
	std::string	server_address;	
	
	// memory handlers
	struct ibv_mr *send_message_mr;
	struct ibv_mr *locks_mr;
	
	// struct ibv_mr *send_data_mr;
	// struct ibv_mr *recv_data_mr;
	

	// memory buffers
	struct BMemoryKeys send_message_msg;
	
	uint64_t* locks;
	
	// char *send_data_msg;
	// int recv_data_msg;
	
	
	int register_memory();
	int destroy_context ();
};
#endif // B_SERVER_CONTEXT_H_