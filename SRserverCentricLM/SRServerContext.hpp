//
//  SRServerContext.h
//  Lock Manager
//
//  Created by Yeounoh Chung on 5/1/15.
//  Copyright (c) 2015 Yeounoh Chung. All rights reserved.
//

#ifndef __Lock_Manager__ServerContext__
#define __Lock_Manager__ServerContext__

#include <stdio.h>
#include "../lock/LockRequest.h"
#include "../lock/LockResponse.h"
#include "SRClientContext.hpp"
#include "../util/BaseContext.hpp"
#include "../config.hpp"

class SRServerContext : public BaseContext {
public:
    int trx_num;
    
    // SRClientContext		client_ctx;
	
	int op_num; //operations (e.g., lock acquisition)
	
	struct	ibv_cq *cq_send;				/* CQ handle for SEND queue */
	struct	ibv_cq *cq_receive;				/* CQ handle for RECEIVE queue */
	
	struct	ibv_comp_channel *comp_channel_send;		/* CQ channel */
	struct	ibv_comp_channel *comp_channel_receive;		/* CQ channel */
		
    
	std::string client_ip;
	int client_port;
	
	// Memory handlers
	struct ibv_mr *lock_req_mr;			
	struct ibv_mr *lock_res_mr;	
	
	// Memory buffers
	struct LockRequest lock_request;
	struct LockResponse lock_response;
	
	int create_context();
    int register_memory ();
    int destroy_context ();
};

#endif /* defined(__Lock_Manager__ServerContext__) */
