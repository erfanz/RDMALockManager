/*
 *	utils.cpp
 *
 *	Created on: 26.Jan.2015
 *	Author: erfanz
 */

#include "utils.hpp"
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <iostream>
#include <netinet/tcp.h>	// for setsockopt()
#include <sched.h>	// for sched_setaffinity() and CPU_SET() and CPU_ZERO()
 

int pin_to_CPU (int CPU_num){
	cpu_set_t  mask;
	CPU_ZERO(&mask);			// Clears set, so that it contains no CPUs.
	CPU_SET(CPU_num, &mask);	// Add CPU 'cpu_num' to set. 
	TEST_NZ(sched_setaffinity(0, sizeof(mask), &mask));	// 0 for the first parameter means the current process
	DEBUG_COUT("Current Process is pinned to core #" << CPU_num);
	return 0;
}


// http://www.concentric.net/~Ttwang/tech/inthash.htm
unsigned long generate_random_seed()
{
	unsigned long a = clock();
	unsigned long b = time(NULL);
	unsigned long c = getpid();
	
    a=a-b;  a=a-c;  a=a^(c >> 13);
    b=b-c;  b=b-a;  b=b^(a << 8);
    c=c-a;  c=c-b;  c=c^(b >> 13);
    a=a-b;  a=a-c;  a=a^(c >> 12);
    b=b-c;  b=b-a;  b=b^(a << 16);
    c=c-a;  c=c-b;  c=c^(b >> 5);
    a=a-b;  a=a-c;  a=a^(c >> 3);
    b=b-c;  b=b-a;  b=b^(a << 10);
    c=c-a;  c=c-b;  c=c^(b >> 15);
    
	return c;
}

int sock_write(int sock, char *buffer, int xfer_size) {
	int rc;
	rc = write (sock, buffer, xfer_size);
	if (rc < xfer_size) {
		std::cerr << "Failed writing data during sock_sync_data" << std::endl;
		return -1;
	}
	return 0;
}

int sock_read(int sock, char *buffer, int xfer_size) {
	int read_bytes = 0;
	int total_read_bytes = 0;
	
	if (read (sock, buffer, xfer_size) <= 0)
		return -1;
	return 0;
	
	/*
	int read_bytes = 0;
	int total_read_bytes = 0;
	
	while (total_read_bytes < xfer_size) {
		read_bytes = read (sock, buffer, xfer_size);
		if (read_bytes > 0)
			total_read_bytes += read_bytes;
		else break;
	}
	if (read_bytes <= 0)
		return -1;
	else return 0;
	*/
}

int sync_it (int sock, char *local_buffer, int xfer_size) {
	char remote_buffer[xfer_size];
	TEST_NZ (sock_write(sock, local_buffer, xfer_size));
	TEST_NZ (sock_read(sock, remote_buffer, xfer_size));
		
	if (strcmp(remote_buffer, local_buffer) == 0)
		return 0;
	return -1;
}

int sock_sync_data (int sock, int xfer_size, char *local_data, char *remote_data) {
	int rc;
	int read_bytes = 0;
	int total_read_bytes = 0;
	rc = write (sock, local_data, xfer_size);
	if (rc < xfer_size)
		std::cerr << "Failed writing data during sock_sync_data" << std::endl;
	else
		rc = 0;
	while (!rc && total_read_bytes < xfer_size) {
		read_bytes = read (sock, remote_data, xfer_size - total_read_bytes);
		if (read_bytes > 0)
			total_read_bytes += read_bytes;
		else
			rc = read_bytes;
	}
	return rc;
}

int sock_connect (std::string servername, int port) {
	int sockfd, n;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	//TPC: avoid merging of streams
	int flag = 1;
	int opt_return = setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (char *) &flag, sizeof(int));
	if (opt_return < 0) {
		std::cerr << "Could not set sock options" << std::endl;
		return -1;
	}
	
	
	DEBUG_COUT("sock_connect returns " << sockfd);
	if (sockfd < 0){ 
		std::cerr << "ERROR opening socket" << std::endl;
		return -1;
	}
	
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	serv_addr.sin_addr.s_addr = inet_addr((char*)servername.c_str());
	if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0){ 
		std::cerr << "ERROR connecting" << std::endl;
		return -1;
	}
	return sockfd;
}

int establish_tcp_connection(std::string remote_ip, int remote_port, int *sockfd) {
	*sockfd = sock_connect (remote_ip, remote_port);
	if (*sockfd < 0) {
		std::cerr << "failed to establish TCP connection to server " <<  remote_ip << " port " << remote_port << std::endl;
		return -1;
	}
	DEBUG_COUT("TCP connection established on sock " << *sockfd);
	return 0;
}

int server_socket_setup(int *server_sockfd, int backlog) {
	struct sockaddr_in serv_addr;
	
	// Open Socket
	*server_sockfd = socket (AF_INET, SOCK_STREAM, 0);
	if (server_sockfd < 0) {
		std::cerr << "Error opening socket" << std::endl;
		return -1;
	}
	
	// Bind
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(TRX_MANAGER_TCP_PORT);
	TEST_NZ(bind(*server_sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)));
	
	// listen				  
	TEST_NZ(listen (*server_sockfd, backlog));
	return 0;
}

void die(const char *reason)
{
	std::cerr << reason << std::endl;
	std::cerr << "Errno: " << strerror(errno) << std::endl;
	exit(EXIT_FAILURE);
}

int load_tables_from_files(ItemVersion* items_region) {
	FILE * fp;
	char * line = NULL;
	size_t len = 0;
	ssize_t read;
	
	// First load ITEM file
	try {
		fp = fopen(ITEM_FILENAME, "r");
		if (fp == NULL){
			std::cerr << "Cannot open file: " << ITEM_FILENAME << std::endl;
			exit(EXIT_FAILURE);
		}		
		int i = 0;
		
		while ((read = getline(&line, &len, fp)) != -1) {
			if (i >= ITEM_CNT)
				// we don't want to read more data from file.
				break;
			
			items_region[i].item.I_ID = atoi(strtok(line,  "\t")) - 1;		// because we want to start IDs from zero
			strcpy(items_region[i].item.I_TITLE,strtok(NULL, "\t"));
			items_region[i].item.I_A_ID = atoi(strtok(NULL, "\t"));
			strcpy(items_region[i].item.I_PUB_DATE, strtok(NULL, "\t"));
			strcpy(items_region[i].item.I_PUBLISHER, strtok(NULL, "\t"));
			strcpy(items_region[i].item.I_SUBJECT, strtok(NULL, "\t"));
			strcpy(items_region[i].item.I_DESC, strtok(NULL, "\t"));
			items_region[i].item.I_RELATED1 = atoi(strtok(NULL, "\t"));
			items_region[i].item.I_RELATED2 = atoi(strtok(NULL, "\t"));
			items_region[i].item.I_RELATED3 = atoi(strtok(NULL, "\t"));
			items_region[i].item.I_RELATED4 = atoi(strtok(NULL, "\t"));
			items_region[i].item.I_RELATED5 = atoi(strtok(NULL, "\t"));
			strcpy(items_region[i].item.I_THUMBNAIL, strtok(NULL, "\t"));
			strcpy(items_region[i].item.I_IMAGE, strtok(NULL, "\t"));
			items_region[i].item.I_SRP = atof(strtok(NULL, "\t"));
			items_region[i].item.I_COST = atof(strtok(NULL, "\t"));
			strcpy(items_region[i].item.I_AVAIL, strtok(NULL, "\t"));
			items_region[i].item.I_STOCK = atoi(strtok(NULL, "\t"));
			strcpy(items_region[i].item.I_ISBN, strtok(NULL, "\t"));
			items_region[i].item.I_PAGE = atoi(strtok(NULL, "\t"));
			strcpy(items_region[i].item.I_BACKING, strtok(NULL, "\t"));
			strcpy(items_region[i].item.I_DIMENSION, strtok(NULL, "\t"));
			
			items_region[i].write_timestamp = 0;	
			i++;
		}
		fclose(fp);
	}
	catch (std::exception& e){
	    std::cerr << "exception caught: " << e.what() << std::endl;
	}	
	if (line)
		free(line);
	
	return 0;
}