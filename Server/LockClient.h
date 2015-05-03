/*
 *	LockClient.hpp
 *
 *	Created on: 21.Feb.2015
 *	Author: erfanz
 *	Modified: yeounoh (1.May.2015)
 */

#ifndef IP_LOCK_CLIENT_H_
#define IP_LOCK_CLIENT_H_

#include "../config.hpp"
#include "ClientContext.h"
#include "../util/utils.hpp"
#include "../lock/LockRequest.h"
#include "../lock/LockResponse.h"
#include <byteswap.h>
#include <stdint.h>
#include <stdlib.h>
#include <infiniband/verbs.h>


class LockClient {
protected:
	int select_item (struct LockRequest &req);
	
private:
	int start_operation(ClientContext &ctx);	

public:
	static void usage (const char *argv0);
	int start_client ();
	int acquire_lock (ClientContext &ctx, struct LockRequest &req, struct LockResponse &res);
	int release_lock (ClientContext &ctx, struct LockRequest &req, struct LockResponse &res);
};

#endif /* IP_LOCK_CLIENT_H_ */
