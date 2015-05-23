/*
 *	CombinedMemoryKeys.hpp
 *
 *	Created on: 19.Feb.2015
 *	Author: erfanz
 */

#ifndef COMBINED_MEMORY_KEYS_H_
#define COMBINED_MEMORY_KEYS_H_

#include <infiniband/verbs.h>
#include <stdint.h>

struct CombinedMemoryKeys {
	struct ibv_mr	peer_item_lock_mr;

	struct ibv_mr	peer_outstanding_mr;	// the remote memory handler for registering SHARED and EXCLUSIVE lock requests (not RELEASE)
	
	uint32_t		client_id;
};


#endif /* COMBINED_MEMORY_KEYS_H_ */