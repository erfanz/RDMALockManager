/*
 *	MemoryKeys.hpp
 *
 *	Created on: 19.Feb.2015
 *	Author: erfanz
 */

#ifndef MEMORY_KEYS_H_
#define MEMORY_KEYS_H_

#include <infiniband/verbs.h>
#include <stdint.h>

struct MemoryKeys {
	struct ibv_mr	peer_mr;
	uint32_t		client_id;
};


#endif /* MEMORY_KEYS_H_ */