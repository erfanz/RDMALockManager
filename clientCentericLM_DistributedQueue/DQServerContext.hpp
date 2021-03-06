/*
 *	DQServerContext.hpp
 *
 *	Created on: 26.Mar.2015
 *	Author: erfanz
 */

#ifndef DQ_SERVER_CONTEXT_H_
#define DQ_SERVER_CONTEXT_H_

#include "../util/BaseContext.hpp"
#include "MemoryKeys.hpp"
#include "../util/RDMACommon.hpp"
#include <iostream>
#include <string>

class DQServerContext : public BaseContext {
public:
	std::string	server_address;	
	
	std::string client_ip;	
	
	// memory handlers
	struct ibv_mr *send_message_mr;
	struct ibv_mr *locks_mr;
	
	// struct ibv_mr *send_data_mr;
	// struct ibv_mr *recv_data_mr;
	

	// memory buffers
	struct MemoryKeys send_message_msg;
	
	uint64_t* locks;
	
	// char *send_data_msg;
	// int recv_data_msg;
	
	//int create_context ();
	
	int register_memory();
	int destroy_context ();
};
#endif // DQ_SERVER_CONTEXT_H_