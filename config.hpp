/*
 *	config.hpp
 *
 *  Created on: 27.Jan.2015
 *	Author: erfanz
 */

#ifndef CONFIG_HPP_
#define CONFIG_HPP_

#include <stdint.h>
#include <string>

#define DEBUG_ENABLED (false)
static const int	CLIENTS_CNT 	= 6;


static const std::string	SERVER_ADDR		= "192.168.0.1";	// only relevant for Tranditional-SI
static const uint16_t		SERVER_TCP_PORT	= 45677;			// only relevant for Tranditional-SI
static const int			SERVER_IB_PORT		= 1;	// only relevant for Tranditional-SI

static const int	OPERATIONS_CNT 		= 10000;

static const int	ITEM_CNT				= 10;		// Number of Items

static const int	LOCK_OWNERSHIP_MEAN_USEC		= 0;	// the mean time to own a lock in micro sec  ;
static const int	LOCK_OWNERSHIP_VARIANCE_USEC	= 0;	// the variance of time of holding a lock

static const int 	SHARED_TO_MIX_RATIO		= 0.5;	// [0,1], 0- all exclusive, 1- all shared

static const int	TIMEOUT_IN_MS			= 500;		/* ms */
static const int	BACK_OFF_MICRO_SEC		= 5;

static const int	MAXIMUM_ATTEMPTS	= 20;		// # of attemps for RDMA lock client before giving up

#endif /* CONFIG_H_ */
