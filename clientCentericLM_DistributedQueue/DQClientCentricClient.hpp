/*
 *	DQClientCentricClient.hpp
 *
 *	Created on: 1.May.2015
 *	Author: erfanz
 */

#ifndef DQ_CLIENT_CENTRIC_CLIENT_H_
#define DQ_CLIENT_CENTRIC_CLIENT_H_

#include <byteswap.h>
#include <stdint.h>
#include <stdlib.h>
#include <infiniband/verbs.h>

#include "../util/RDMACommon.hpp"
#include "DQClientContext.hpp"

class DQClientCentricClient{
private:
	static int start_benchmark(DQClientContext &ctx);
	static bool request_shared_lock(DQClientContext &ctx, int lock_id);
	static bool request_exclusive_lock(DQClientContext &ctx, int lock_id);
	static int release_shared_lock(DQClientContext &ctx, int lock_id);
	static int release_exclusive_lock(DQClientContext &ctx, int lock_id);
	static bool re_inquire_shared_lock(DQClientContext &ctx, int lock_id);
	

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

#endif /* DQ_CLIENT_CENTRIC_CLIENT_H_ */