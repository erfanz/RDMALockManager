/*
 *	CombiendClient.hpp
 *
 *	Created on: 1.May.2015
 *	Author: erfanz
 */

#ifndef COMBINED_CLIENT_H_
#define COMBINED_CLIENT_H_

#include <byteswap.h>
#include <stdint.h>
#include <stdlib.h>
#include <infiniband/verbs.h>

#include "../util/RDMACommon.hpp"
#include "CombinedClientContext.hpp"



class CombinedClient{
private:
	static int select_item (struct LockRequest &req);
	static int hold_lock ();
	static int start_benchmark(CombinedClientContext &ctx);
	static int acquire_shared_lock(CombinedClientContext &ctx);
	static int acquire_exclusive_lock(CombinedClientContext &ctx);
	static bool try_acquire_shared_lock(CombinedClientContext &ctx);
	static bool try_acquire_exclusive_lock(CombinedClientContext &ctx);
	static int register_request_as_outstanding(CombinedClientContext &ctx);	
	static int release_lock (CombinedClientContext &ctx);
	
	

public:
	/******************************************************************************
	* Function: start_client
	*
	* Input
	* nothing
	*
	* Returns
	* socket (fd) on success, negative error code on failure
	*
	* Description
	* Starts the client. 
	*
	******************************************************************************/
	int start_client ();
	
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
};

#endif /* COMBINED_CLIENT_H_ */