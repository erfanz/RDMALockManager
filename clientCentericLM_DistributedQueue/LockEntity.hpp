/*
 *	LockEntity.hpp
 *
 *	Created on: 26.Mar.2015
 *	Author: erfanz
 */

#ifndef LOCK_ENTITY_H_
#define LOCK_ENTITY_H_

#include <stdint.h>

class LockEntity {
	uint32_t shared_tail;
	uint32_t exclusive_tail;
};

#endif // LOCK_ENTITY_H_