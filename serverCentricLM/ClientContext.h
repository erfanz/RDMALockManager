//
//  ClientContext.h
//  ServerCentricLM
//
//  Created by Yeounoh Chung on 5/1/15.
//

#ifndef __Lock_Manager__ClientContext__
#define __Lock_Manager__ClientContext__

#include <stdio.h>
#include "../util/BaseContext.hpp"
#include "../config.hpp"

class ClientContext : public BaseContext {
public:
	int op_num; //operations (e.g., lock acquisition)
    
	std::string client_ip;
	int client_port;

	//struct ibv_* defined in BaseContext 
	//int sockfd defined in BaseContext
	
	int create_context ();
	int register_memory (); //abstract func in BaseContext
	int destroy_context (); //abstract func in BaseContext
};

#endif
