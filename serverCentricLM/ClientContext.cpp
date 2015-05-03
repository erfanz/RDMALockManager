//
//  ClientContext.cpp
//  Lock Manager
//
//  Created by Yeounoh Chung on 5/1/15.
//  Copyright (c) 2015 Yeounoh Chung. All rights reserved.
//

#include "ClientContext.h"
#include "../util/utils.hpp"
#include <unistd.h>		// for close
#include <iostream>

int ClientContext::create_context() {
    return 0;
}

int ClientContext::register_memory() {
    return 0;
}

int ClientContext::destroy_context () {
    if (sockfd >= 0)	TEST_NZ (close (sockfd));
    return 0;
}