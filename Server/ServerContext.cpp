//
//  ServerContext.cpp
//  Lock Manager
//
//  Created by Yeounoh Chung on 5/1/15
//  from IPTradTrxManagerContext.cpp
//  Copyright (c) 2015 Yeounoh Chung. All rights reserved.
//

#include "ServerContext.h"
#include "../util/utils.hpp"
#include <unistd.h>		// for close
#include <iostream>

// Override
int ServerContext::create_context() {
    return 0;
}

int ServerContext::register_memory() {
    return 0;
}

int ServerContext::destroy_context () {
    TEST_NZ (client_ctx.destroy_context());
    return 0;
}