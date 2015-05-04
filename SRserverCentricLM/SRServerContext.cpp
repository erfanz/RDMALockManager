//
//  SRServerContext.cpp
//  Lock Manager
//
//  Created by Yeounoh Chung on 5/1/15
//  from IPTradTrxManagerContext.cpp
//  Copyright (c) 2015 Yeounoh Chung. All rights reserved.
//

#include "SRServerContext.hpp"
#include "../util/utils.hpp"
#include <unistd.h>		// for close
#include <iostream>


int SRServerContext::register_memory() {
	int mr_flags =
		IBV_ACCESS_LOCAL_WRITE
			| IBV_ACCESS_REMOTE_READ
				| IBV_ACCESS_REMOTE_WRITE
					| IBV_ACCESS_REMOTE_ATOMIC;
	
	TEST_Z(lock_req_mr	= ibv_reg_mr(pd, &lock_request, sizeof(struct LockRequest), mr_flags));
	TEST_Z(lock_res_mr	= ibv_reg_mr(pd, &lock_response, sizeof(struct LockResponse), mr_flags));
	DEBUG_COUT("[Info] Memory registered");
	return 0;
}

int SRServerContext::destroy_context () {
	if (qp) 			TEST_NZ(ibv_destroy_qp (qp));
	if (lock_req_mr)	TEST_NZ (ibv_dereg_mr (lock_req_mr));
	if (lock_res_mr)	TEST_NZ (ibv_dereg_mr (lock_res_mr));
	if (cq)				TEST_NZ (ibv_destroy_cq (cq));
	if (pd)				TEST_NZ (ibv_dealloc_pd (pd));
	if (ib_ctx)			TEST_NZ (ibv_close_device (ib_ctx));
	if (sockfd >= 0)	TEST_NZ (close (sockfd));
	
    return 0;
}