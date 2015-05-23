/*
 *	LockRequest.h
 *	Lock
 *
 *	Created by Yeounoh Chung on 5/1/15.
 */

#ifndef _LOCK_REQUEST_H_
#define _LOCK_REQUEST_H_

struct LockRequest {
	int request_item;
	
	enum RequestType {
		SHARED,
		EXCLUSIVE,
		RELEASE,
		RELEASE_SHARED,
		RELEASE_EXCLUSIVE,		
		EMPTY
	} request_type;
};

#endif /* _LOCK_REQUEST_H_ */