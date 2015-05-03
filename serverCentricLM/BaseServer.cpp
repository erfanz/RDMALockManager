//
//  BaseServer.cpp
//  Lock Manager
//
//  Created by Yeounoh Chung on 5/1/15 based on BaseTradTrxManager.cpp by erfanz.
//  Copyright (c) 2015 Yeounoh Chung. All rights reserved.
//

#include "BaseServer.h"
#include "../util/utils.hpp"
#include "../util/BaseContext.hpp"
#include <stdio.h>
#include <string.h>
#include <sstream>	// for stringstream
#include <unistd.h>
#include <inttypes.h>
#include <endian.h>
#include <getopt.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <iostream>

int	BaseServer::server_sockfd	= -1;		// Server's socket file descriptor

std::string BaseServer::get_full_desc(BaseContext &ctx) {
    std::stringstream sstm;
    sstm << ctx.sockfd;
    return sstm.str();
}

int BaseServer::destroy_resources () {
    close(server_sockfd);	// close the socket
    return 0;
}

void BaseServer::usage (const char *argv0) {
    std::cout << "Usage:" << std::endl;
    std::cout << argv0 << " starts a server and wait for connection" << std::endl;
}