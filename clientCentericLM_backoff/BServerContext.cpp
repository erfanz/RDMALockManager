/*
 *	BServerContext.cpp
 *
 *	Created on: 26.Mar.2015
 *	Author: erfanz
 */

#include "BServerContext.hpp"
#include "../util/utils.hpp"
#include "../config.hpp"

#include <unistd.h>		// for close
#include <iostream>


int BServerContext::register_memory() {
	int mr_flags	=
		IBV_ACCESS_LOCAL_WRITE
			| IBV_ACCESS_REMOTE_READ
				| IBV_ACCESS_REMOTE_WRITE
					| IBV_ACCESS_REMOTE_ATOMIC;
			

	TEST_Z(send_message_mr	= ibv_reg_mr(pd, &send_message_msg, sizeof(struct BMemoryKeys), mr_flags));
	TEST_Z(locks_mr			= ibv_reg_mr(pd, locks, ITEM_CNT * sizeof(uint64_t), mr_flags));
	
	return 0;
}

int BServerContext::destroy_context () {
	if (qp)
		TEST_NZ(ibv_destroy_qp (qp));
	
	if (send_message_mr) TEST_NZ (ibv_dereg_mr (send_message_mr));
	if (locks_mr) TEST_NZ (ibv_dereg_mr (locks_mr));
	// if (send_data_mr) TEST_NZ (ibv_dereg_mr (send_data_mr));
	// if (recv_data_mr) TEST_NZ (ibv_dereg_mr (recv_data_mr));
	
	
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