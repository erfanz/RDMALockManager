/*
 *	BServerContext.cpp
 *
 *	Created on: 26.Mar.2015
 *	Author: erfanz
 */

#include "CombinedServerContext.hpp"
#include "../lock/LockRequest.h"
#include "../util/utils.hpp"
#include "../config.hpp"

#include <unistd.h>		// for close
#include <iostream>


int CombinedServerContext::register_memory() {
	int mr_flags	=
		IBV_ACCESS_LOCAL_WRITE
			| IBV_ACCESS_REMOTE_READ
				| IBV_ACCESS_REMOTE_WRITE
					| IBV_ACCESS_REMOTE_ATOMIC;
			
	TEST_Z(send_message_mr	= ibv_reg_mr(pd, &send_message_msg, sizeof(struct CombinedMemoryKeys), mr_flags));
	TEST_Z(item_lock_mr		= ibv_reg_mr(pd, item_lock_array, ITEM_CNT * sizeof(uint64_t), mr_flags));
	TEST_Z(outstanding_mr	= ibv_reg_mr(pd, outstanding_array, CLIENTS_CNT * sizeof(struct LockRequest), mr_flags));
	TEST_Z(lock_res_mr		= ibv_reg_mr(pd, &lock_response, sizeof(struct LockResponse), mr_flags));
	
	return 0;
}

int CombinedServerContext::destroy_context () {
	if (qp)
		TEST_NZ(ibv_destroy_qp (qp));
	
	if (send_message_mr) TEST_NZ (ibv_dereg_mr (send_message_mr));
	if (item_lock_mr) TEST_NZ (ibv_dereg_mr (item_lock_mr));
	if (outstanding_mr) TEST_NZ (ibv_dereg_mr (outstanding_mr));
	if (lock_res_mr) TEST_NZ (ibv_dereg_mr (lock_res_mr));
	
	if (cq)
		TEST_NZ (ibv_destroy_cq (cq));
	
	if (pd)
		TEST_NZ (ibv_dealloc_pd (pd));
	
	if (ib_ctx)
		TEST_NZ (ibv_close_device (ib_ctx));

	if (sockfd >= 0){
		TEST_NZ (close (sockfd));
	}
}