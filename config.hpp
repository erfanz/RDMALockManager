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


#define DEBUG_ENABLED (true)
static const int	CLIENTS_CNT 	= 1;


static const std::string	SERVER_ADDR		= "192.168.0.1";	// only relevant for Tranditional-SI
static const uint16_t		SERVER_TCP_PORT	= 45677;			// only relevant for Tranditional-SI
static const int			SERVER_IB_PORT		= 1;	// only relevant for Tranditional-SI

static const int	OPERATIONS_CNT 		= 100000;

static const int	ITEM_CNT				= 100000;		// Number of Items

static const int	TIMEOUT_IN_MS			= 500;		/* ms */

#endif /* CONFIG_H_ */
