/*
 *	BaseContext.hpp
 *
 *	Created on: 19.Feb.2015
 *	Author: erfanz
 */

#ifndef BASE_CONTEXT_H_
#define BASE_CONTEXT_H_


#include <infiniband/verbs.h>


class BaseContext {
public:
	struct	ibv_device_attr device_attr;
	struct	ibv_port_attr port_attr;			/* IB port attributes */
	struct	ibv_context *ib_ctx;				/* device handle */
	struct	ibv_pd *pd;							/* PD handle */
	struct	ibv_cq *cq;							/* CQ handle */
	struct	ibv_qp *qp;							/* QP handle */
	struct	ibv_comp_channel *comp_channel;		/* CQ channel */
	
	int		ib_port;
	int 	sockfd = -1;			
	
	
	/******************************************************************************
	* Function: create_context
	*
	* Input
	* none
	*
	* Returns
	* 0 on success, 1 on failure
	*
	* Description
	*
	* Creates and allocates all necessary system Context.
	*****************************************************************************/
	int create_context();
	
	virtual int register_memory() = 0;
	
	/******************************************************************************
	* Function: destroy_context
	*
	* Input
	* none
	*
	* Returns
	* 0 on success, 1 on failure
	*
	* Description
	* Cleanup and deallocate all Context resources
	******************************************************************************/
	virtual int destroy_context () = 0;
};
#endif /* BASE_CONTEXT_H_ */