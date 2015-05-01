//
//  ServerContext.h
//  Lock Manager
//
//  Created by Yeounoh Chung on 5/1/15.
//  Copyright (c) 2015 Yeounoh Chung. All rights reserved.
//

#ifndef __Lock_Manager__ServerContext__
#define __Lock_Manager__ServerContext__

#include <stdio.h>
#include "../util/BaseContext.hpp"
#include "ClientContext.h"
#include "config.hpp"

class ServerContext : public BaseContext {
public:
    int trx_num;
    
    ClientContext		client_ctx;
    
    int create_context ();
    
    int register_memory ();
    
    int destroy_context ();
};

#endif /* defined(__Lock_Manager__ServerContext__) */
