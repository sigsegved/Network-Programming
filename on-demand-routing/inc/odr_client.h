#ifndef __ODR_CLIENT_H
#define __ODR_CLIENT_H
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>

#define MAX_MESSAGE_SIZE 100

#define EVENT_SEND 1
#define EVENT_BIND 2

#define RECV_TIMEOUT 1
#define RECV_OK 0
#define RECV_ERROR -1

#define SERVER_DOMAIN_FILE "server.sock"

typedef struct {
	int event;
	char ip[20];
	int src_port;
	int dst_port;
	char msg[MAX_MESSAGE_SIZE];
	int flags;
	
} odr_client_com;


// Returns the socket descriptor
// 0 as the port number will result in a random port being selected
int client_init(int port);
void client_cleanup(int socketfd);
void msg_send(int socket, char* ip, int port, char* msg, int flags);
int msg_recv(int socket, char* ip, int* port, char* msg);


// Blocking, returns on event from client and also returns their address.
int server_bind();
int server_getevent(int sock, odr_client_com *data);

// Sends an event to client, given the client address
void server_sendevent(int socketfd, char* sourceip, int sourceport, int destport, char* msg);

#endif
