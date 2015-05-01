//
//  LockServer.h
//  Lock Manager
//
//  Created by Yeounoh Chung on 5/1/15.
//  Copyright (c) 2015 Yeounoh Chung. All rights reserved.
//

#ifndef __Lock_Manager__LockServer__
#define __Lock_Manager__LockServer__

#include <stdio.h>
#include "ServerContext.h"
#include "BaseServer.h"

class LockServer : public BaseServer {
private:
    static void*	handle_client(void *param);
    
    static int		start_transactions(ServerContext &ctx);
    
public:
    int start_server ();
    
};


#endif /* defined(__Lock_Manager__LockServer__) */
