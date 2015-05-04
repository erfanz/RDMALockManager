//
//  SRLockServer.h
//  Lock Manager
//
//  Created by Yeounoh Chung on 5/1/15.
//

#ifndef __SR_Lock_Manager__LockServer__
#define __SR_Lock_Manager__LockServer__

#include <stdio.h>
#include "SRServerContext.hpp"
#include "../config.hpp"
#include "../util/utils.hpp"
#include "../lock/LockRequest.h"
#include "../lock/LockResponse.h"
#include <arpa/inet.h>
#include <stdint.h>
#include <stdlib.h>
#include <queue>
#include <list>
#include <vector>
#include <arpa/inet.h>
#include <infiniband/verbs.h>
#include <mutex>

struct QueuedRequest {
	SRServerContext *ctx;

	struct LockRequest req;
};

class SRLockServer { //not extending BaseServer (removed)
protected:
	int server_sockfd;
	
	static int register_request (SRServerContext &ctx, struct LockRequest &req, struct LockResponse &res);
	static int grant_shared_locks (SRServerContext &ctx, struct LockRequest &req, struct LockResponse &res);
	int destroy_resources ();
	
private:
	
	static std::list <QueuedRequest> list_array[ITEM_CNT]; //for array[const size], don't need to clean up
	static std::mutex list_mutex[ITEM_CNT];
		
    static void* handle_client(void *param);
    static int start_operation (SRServerContext &ctx);
    
public:
    int start_server ();
	static void usage (const char *argv0);
};


#endif /* defined(__SR_Lock_Manager__LockServer__) */
