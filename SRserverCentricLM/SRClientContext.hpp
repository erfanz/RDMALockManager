//
//  SRClientContext.h
//  ServerCentricLM
//
//  Created by Yeounoh Chung on 5/1/15.
//

#ifndef __Lock_Manager__ClientContext__
#define __Lock_Manager__ClientContext__

#include <stdio.h>
#include "../lock/LockRequest.h"
#include "../lock/LockResponse.h"
#include "../util/BaseContext.hpp"
#include "../config.hpp"

class SRClientContext : public BaseContext {
public:
	int op_num; //operations (e.g., lock acquisition)
    
	std::string client_ip;
	int client_port;
	
	// Memory handlers
	struct ibv_mr *lock_req_mr;			
	struct ibv_mr *lock_res_mr;	
	
	// Memory buffers
	struct LockRequest lock_request;
	struct LockResponse lock_response;
	
	int register_memory (); //abstract func in BaseContext
	int destroy_context (); //abstract func in BaseContext
};

#endif
