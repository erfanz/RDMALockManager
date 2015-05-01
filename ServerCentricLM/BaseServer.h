//
//  BaseServer.h
//  Lock Manager
//
//  Created by Yeounoh Chung on 5/1/15.
//  Copyright (c) 2015 Yeounoh Chung. All rights reserved.
//

#ifndef __Lock_Manager__BaseServer__
#define __Lock_Manager__BaseServer__

#include <stdio.h>
#include "../config.hpp"
#include <stdint.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <infiniband/verbs.h>

class BaseServer {
protected:
    static int			server_sockfd;		// Server's socket file descriptor
    
    static std::string get_full_desc(BaseContext &ctx); // return socketfd
    
    int destroy_resources (); // close socketfd
    
public:
    static void usage (const char *argv0);
};
#endif /* defined(__Lock_Manager__BaseServer__) */
