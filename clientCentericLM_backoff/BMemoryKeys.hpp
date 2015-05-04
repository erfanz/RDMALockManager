/*
 *	BMemoryKeys.hpp
 *
 *	Created on: 19.Feb.2015
 *	Author: erfanz
 */

#ifndef B_MEMORY_KEYS_H_
#define B_MEMORY_KEYS_H_

#include <infiniband/verbs.h>
#include <stdint.h>

struct BMemoryKeys {
	struct ibv_mr	peer_mr;
	uint32_t		client_id;
};


#endif /* B_MEMORY_KEYS_H_ */