/*
 *	CombinedServer.hpp
 *
 *	Created on: 25.Jan.2015
 *	Author: erfanz
 */

#ifndef COMBINED_SERVER_H_
#define COMBINED_SERVER_H_

#include "CombinedServerContext.hpp"
#include "../util/RDMACommon.hpp"

#include <stdint.h>
#include <stdlib.h>
#include <infiniband/verbs.h>


class CombinedServer{
private:
	int	server_sockfd;		// Server's socket file descriptor
	int	tcp_port;
	int	ib_port;
	
	// memory buffers
	//std::atomic<uint64_t> *item_lock_array;
	std::atomic<uint64_t> item_lock_array[ITEM_CNT];
	
	LockRequest *outstanding_array;
	
	
	void convert(uint64_t x);
	
	
	int start_operations(CombinedServerContext *ctx);
	
	int initialize_data_structures();
	int initialize_context(CombinedServerContext &ctx);
	
	int handle_shared_lock(CombinedServerContext &ctx);

	int handle_exclusive_lock(CombinedServerContext &ctx);

	int handle_release_lock(CombinedServerContext &ctx);
public:
	
	
	
	/******************************************************************************
	* Function: start_server
	*
	* Input
	* server_number (e.g. 0, 1, ...., Config.SERVER_CNT)
	*
	* Returns
	* socket (fd) on success, negative error code on failure
	*
	* Description
	* Starts the server. 
	*
	******************************************************************************/
	int start_server ();
	
	/******************************************************************************
	* Function: usage
	*
	* Input
	* argv0 command line arguments
	*
	* Output
	* none
	*
	* Returns
	* none
	*
	* Description
	* print a description of command line syntax
	******************************************************************************/
	static void usage (const char *argv0);
	
	~CombinedServer ();
	
};
#endif /* COMBINED_SERVER_H_ */