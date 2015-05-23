/*
 *	CombinedClientContext.cpp
 *
 *	Created on: 26.Mar.2015
 *	Author: erfanz
 */

#include "CombinedClientContext.hpp"
#include "../util/utils.hpp"
#include <unistd.h>		// for close
#include <iostream>


int CombinedClientContext::register_memory() {
	int mr_flags = 
		IBV_ACCESS_LOCAL_WRITE
			| IBV_ACCESS_REMOTE_READ
				| IBV_ACCESS_REMOTE_WRITE;
	
	TEST_Z(recv_memory_mr	= ibv_reg_mr(pd, &recv_memory_msg, 	sizeof(struct CombinedMemoryKeys), mr_flags));
	TEST_Z(item_lock_mr		= ibv_reg_mr(pd, &item_lock, 		sizeof(uint64_t), mr_flags));
	TEST_Z(lock_req_mr		= ibv_reg_mr(pd, &lock_request, 	sizeof(struct LockRequest), mr_flags));		
	TEST_Z(lock_res_mr		= ibv_reg_mr(pd, &lock_response, 	sizeof(struct LockResponse), mr_flags));
	
	return 0;
}

int CombinedClientContext::destroy_context () {
	if (qp)				TEST_NZ(ibv_destroy_qp (qp));
	if (recv_memory_mr)		TEST_NZ (ibv_dereg_mr (recv_memory_mr));	
	if (item_lock_mr)		TEST_NZ (ibv_dereg_mr (item_lock_mr));
	if (lock_req_mr)		TEST_NZ (ibv_dereg_mr (lock_req_mr));	
	if (lock_res_mr)		TEST_NZ (ibv_dereg_mr (lock_res_mr));
		
	if (cq)						TEST_NZ (ibv_destroy_cq (cq));
	if (pd)						TEST_NZ (ibv_dealloc_pd (pd));
	if (ib_ctx)					TEST_NZ (ibv_close_device (ib_ctx));
	if (sockfd >= 0)			TEST_NZ (close (sockfd));
	
	return 0;
}