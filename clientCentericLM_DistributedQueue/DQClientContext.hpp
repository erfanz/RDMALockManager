/*
 *	DQClientContext.hpp
 *
 *	Created on: 26.Mar.2015
 *	Author: erfanz
 */

#ifndef DQ_CLIENT_CONTEXT_H_
#define DQ_CLIENT_CONTEXT_H_

#include "../util/BaseContext.hpp"
#include "../util/RDMACommon.hpp"
#include "MemoryKeys.hpp"
#include <iostream>
#include <vector>

class DQClientContext : public BaseContext{
public:
	std::string	server_address;
	
	uint32_t					client_identifier;		// this will be provided by the server
	std::vector<std::string>	clients_addr_vector;		// the IP address of all clients (including self)
	std::vector<std::string>	clients_tcp_port_vector;	// the TCP port of all clients (including self)
	
	// local memory handler
	struct ibv_mr *recv_memory_mr;			
	struct ibv_mr *lock_result_mr;	
	
	// memory bufferes
	struct MemoryKeys	recv_memory_msg;
	uint64_t lock_result;
	
	// remote memory handlers
	struct ibv_mr peer_data_mr;
	
	// int create_context ();
	int register_memory();
	int destroy_context ();
};
#endif // DQ_CLIENT_CONTEXT_H_