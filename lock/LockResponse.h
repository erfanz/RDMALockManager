/*
 *	LockResponse.h
 *	lock
 *
 *	Created by Yeounoh Chung on 5/1/15.
 */

#ifndef _LOCK_RESPONSE_H_
#define _LOCK_RESPONSE_H_

struct LockResponse {
	enum ResponseType {
		GRANTED,
		RELEASED,
		FAILED
	} response_type;
};

#endif /* _LOCK_RESPONSE_H_ */