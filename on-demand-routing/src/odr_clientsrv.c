#include "odr_client.h"
static struct sockaddr_un srv_name;

typedef struct {
	int port;
	struct sockaddr_un addr;
} portmap;

#define MAX_PORTS 20
portmap ports[MAX_PORTS];

int server_bind()
{
	int i;
	int sockfd = -1;
	if ((sockfd = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0)
	{
		printf("Error creating a datagram socket\n");
		return -1;
	}
	
	// Bind to a file to listen to
	unlink(SERVER_DOMAIN_FILE);
	srv_name.sun_family = AF_UNIX;
	strcpy(srv_name.sun_path, SERVER_DOMAIN_FILE);
	
    if (bind(sockfd, (struct sockaddr *) &srv_name, sizeof(struct sockaddr_un))) 
	{
        printf("Error binding a datagram socket");
		return -1;
    }
	
	
	for (i = 0; (i < MAX_PORTS); i++) {
		ports[i].port = 0;
	}
		
	return sockfd;
}

// Blocking, returns on event from client and also returns their address.
int server_getevent(int socketfd, odr_client_com *data)
{
	char buf[sizeof(odr_client_com)];
	struct sockaddr_un cli_name;
	socklen_t address_length = sizeof(struct sockaddr_un);
	
	int bytes_received = recvfrom(socketfd, buf, sizeof(odr_client_com), 0, 
		(struct sockaddr*)&(cli_name), &address_length);
		
	if (bytes_received != sizeof(odr_client_com))
	{
		printf("\nDatagram size wrong!!\n");
		return -1;
	}
	
	//printf("Received length %d\n", bytes_received);
	
	odr_client_com *recvdata = (odr_client_com *)&buf;
	*data = *recvdata;
	
	int i;
	if (recvdata->event == EVENT_BIND)
	{
		printf("Binding port %d",data->src_port );
		// Look for existing binds to this port and clear them.
		for (i = 0; (i < MAX_PORTS); i++) 
		{
			if (ports[i].port == data->src_port) {
				ports[i].port = 0;
			}
		}
		
		// Look for an empty slot and bind it
		for (i = 0; (i < MAX_PORTS); i++) {
			if (ports[i].port == 0) {
				ports[i].port = data->src_port;
				ports[i].addr = cli_name;
				break;
			}
		}

		return -2;
	}
	else {
		printf("Sending message!\n");
	}
	
	return 0;
}

// Sends an event to client, given the client address
void server_sendevent(int socketfd, char* sourceip, int sourceport, int destport, char* msg)
{
	portmap *port = NULL;
	int i;
	
	// Look for existing binds to this port and clear them.
	for (i = 0; (i < MAX_PORTS); i++) {
		if (ports[i].port == destport) {
			port = &ports[i];
			break;
		}
	}
	
	if (port == NULL) {
		printf("Tried to send to a port that isn't mapped.\n");
		return;
	}
	
	odr_client_com data;
	strcpy(data.ip, sourceip);
	strcpy(data.msg, msg);
	data.src_port = sourceport;
	data.dst_port = destport;
	data.flags = 0;
	data.event = EVENT_SEND;
	
	if (sendto(socketfd, (char *)&data, sizeof(odr_client_com), 0, (struct sockaddr *)&port->addr, sizeof(struct sockaddr_un)) < 0) {
		perror("Error sending datagram message");
		return;
	}
}
