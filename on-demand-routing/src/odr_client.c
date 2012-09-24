#include "odr_client.h"
#include "time.h"

static struct sockaddr_un cli_name;
static int cli_port;

void msg_send(int socketfd, char* ip, int port, char* msg, int flags)
{
	struct sockaddr_un name;
	name.sun_family = AF_UNIX;
	strcpy(name.sun_path, SERVER_DOMAIN_FILE);

	odr_client_com data;
	strcpy(data.ip, ip);
	strcpy(data.msg, msg);
	data.dst_port = port;
	data.src_port = cli_port;
	data.flags = flags;
	data.event = EVENT_SEND;

	if (sendto(socketfd, (char *)&data, sizeof(odr_client_com), 0, (struct sockaddr *)&name, sizeof(struct sockaddr_un)) < 0) {
		printf("Error sending message to ODR, make sure its running in the same folder path.\n");
		client_cleanup(socketfd);
		exit(1);
		return;
	}
}

int msg_recv(int socketfd, char* ip, int* port, char* msg)
{
	char buf[sizeof(odr_client_com)];

	socklen_t address_length = sizeof(struct sockaddr_un);
	int bytes_received;
	
	// Start listening
	struct sockaddr_un srv_name;
	srv_name.sun_family = AF_UNIX;
	strcpy(srv_name.sun_path, SERVER_DOMAIN_FILE);
	
	
	// Use syscall select for timeouts
	fd_set rfds;
	struct timeval tv;
	
	tv.tv_sec = 30;
	tv.tv_usec = 0;
	FD_ZERO(&rfds);
	FD_SET(socketfd, &rfds);
	
	int retval = select(socketfd + 1, &rfds, NULL, NULL, &tv);
	if (retval < 0) {
		printf("Error with select listening\n");
		return RECV_ERROR; 
	}
	
	if (FD_ISSET(socketfd, &rfds)) {
		bytes_received = recvfrom(socketfd, buf, sizeof(odr_client_com), 0, (struct sockaddr*)&(srv_name), &address_length);
		if (bytes_received != sizeof(odr_client_com))
		{
			printf("\nDatagram size wrong!!\n");
			*msg = 0;
			return RECV_ERROR;
		}
	}
	else {
		return RECV_TIMEOUT;
	}
	
	odr_client_com *recvdata = (odr_client_com *)&buf;
	strcpy(msg, recvdata->msg);	
	strcpy(ip, recvdata->ip);	
	*port = recvdata->src_port;
	return RECV_OK;
}

int client_init(int port)
{
	cli_port = port;
	int sockfd = -1;
	if ((sockfd = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0)
	{
		printf("Error creating a datagram socket\n");
		return -1;
	}
	
	// Bind to a file to listen to
	cli_name.sun_family = AF_UNIX;
	strcpy(cli_name.sun_path, "client.XXXXXX");
	
	int fd = mkstemp(cli_name.sun_path);
	if (fd == -1)
	{
		printf("Unable to create temporary file\n");
	}
	unlink(cli_name.sun_path);
	close(fd);
	
	
    if (bind(sockfd, (struct sockaddr *) &cli_name, sizeof(struct sockaddr_un))) {
        printf("Error binding a datagram socket");
		return -1;
    }
	
	
	if (cli_port == 0) {
		// Lets make up a unique port number, its not really well thought out though.
		char* pos = cli_name.sun_path + 7;
		
		cli_port = time(NULL) % 99999; // Replace with 99999 if compile issue.
		
		while (*pos != 0) {
			cli_port = (cli_port * 30) + (*pos - '0');
			pos++;
		}
	}

	struct sockaddr_un srv_name;
	srv_name.sun_family = AF_UNIX;
	strcpy(srv_name.sun_path, SERVER_DOMAIN_FILE);
	
	// Send the ODR a bind message so that it knows where we're listening from.
	odr_client_com data;
	strcpy(data.ip, "");
	data.src_port = cli_port;
	data.flags = 0;
	data.event = EVENT_BIND;
	
	if (sendto(sockfd, (char *)&data, sizeof(odr_client_com), 0, (struct sockaddr *)&srv_name, sizeof(struct sockaddr_un)) < 0) 
	{
		printf("Error sending message to ODR, make sure its running in the same folder path.\n");
		client_cleanup(sockfd);
		exit(1);
		return -1;
	}
	
	return sockfd;
}

void client_cleanup(int socketfd)
{
    close(socketfd);
    unlink(cli_name.sun_path);
}
