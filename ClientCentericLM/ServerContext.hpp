/*
 *	ServerContext.hpp
 *
 *	Created on: 26.Mar.2015
 *	Author: erfanz
 */

#ifndef SERVER_CONTEXT_H_
#define SERVER_CONTEXT_H_

#include "../../util/BaseContext.hpp"
#include "MemoryKeys.hpp"
#include "../../util/RDMACommon.hpp"
#include <iostream>

class ServerContext : public BaseContext {
public:
	std::string	server_address;	
	
	// memory handlers
	struct ibv_mr *send_message_mr;
	struct ibv_mr *local_mr;
	
	struct ibv_mr *send_data_mr;
	struct ibv_mr *recv_data_mr;
	

	// memory buffers
	struct MemoryKeys send_message_msg;
	
	char *local_buffer;
	
	char *send_data_msg;
	int recv_data_msg;
	
	
	int register_memory();
	int destroy_context ();
};
#endif // SERVER_CONTEXT_H_