//
//  ClientContext.h
//  Lock Manager
//
//  Created by Yeounoh Chung on 5/1/15.
//  Copyright (c) 2015 Yeounoh Chung. All rights reserved.
//

#ifndef __Lock_Manager__ClientContext__
#define __Lock_Manager__ClientContext__

#include <stdio.h>
#include "../util/BaseContext.hpp"
#include "../config.hpp"

class ClientContext : public BaseContext {
public:
    int trx_num;
    struct Cart				shopping_cart;
    
    std::string client_ip;
    int client_port;
    
    // memory bufferes
    struct CommitRequest	commit_request;
    struct CommitResponse	commit_response;
    
    int create_context ();
    int register_memory ();
    int destroy_context ();
};

#endif /* defined(__Lock_Manager__ClientContext__) */
