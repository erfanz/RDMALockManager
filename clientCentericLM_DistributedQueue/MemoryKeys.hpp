/*
 *	MemoryKeys.hpp
 *
 *	Created on: 19.Feb.2015
 *	Author: erfanz
 */

#ifndef MEMORY_KEYS_H_
#define MEMORY_KEYS_H_

#include "../config.hpp"
#include <infiniband/verbs.h>
#include <stdint.h>
#include <string>


struct MemoryKeys {
	struct ibv_mr	peer_mr;
	uint32_t		client_id;
	char 			clients_addr[CLIENTS_CNT * 15];		// note that this is one string, so different client IPs are separated by the delimiter '|' 
	char			clients_tcp_port[CLIENTS_CNT * 4];	// note that this is one string, so different client pots are separated by the delimiter '|'
	
	// the constant sizes for clients_addr and clients_tcp_port are ugly. but I saw no way out of them.
	// basically, we don't  strings in our MemoryKey, and since we don't know the size of them before filling them,
	// we have to pessimisticly estimate out much space we need for those
	// each ip is in the from "XXX.XXX.XXX.XXX", so 20 should be sufficient 
	// each port is in the from "XXXXX", so 20 should be sufficient
		
};


#endif /* MEMORY_KEYS_H_ */