#include <stdio.h>
#include "unpifiplus.h"

#define PRINT_ADDRESS(x) Sock_ntop((SA *)x, sizeof(struct sockaddr))


/*char settings_IP[100];
char settings_Port[100];
char settings_Filename[255];
int settings_WindowSize;
int settings_Seed;
float settings_DatagramLoss;
int settings_Rate;
*/

struct sockaddr_in IPclient;
struct in_addr IPserver;
int recvfd;
int local;


// Read the settings of the file
int
read_settings() 
{
	FILE* file = fopen("client.in", "r");
	
	if (!file) {
		return -1;
	}

	memset(settings_IP,0,sizeof(settings_IP));
	memset(settings_Port,0,sizeof(settings_Port));
	memset(settings_Filename,0,sizeof(settings_Filename));
	
	fscanf(file, "%s\n", settings_IP); 
	fscanf(file, "%s\n", settings_Port); 
	fscanf(file, "%s\n", settings_Filename); 
	fscanf(file, "%d\n", &settings_WindowSize); 
	fscanf(file, "%d\n", &settings_Seed); 
	fscanf(file, "%f\n", &settings_DatagramLoss); 
	fscanf(file, "%d\n", &settings_Rate);
	fclose(file);
	
	return 0;
}

// Determine if the server is running on the same machine or not
void
determine_locality() 
{
	local = 0;
	struct ifi_info *data = Get_ifi_info_plus(AF_INET, 1);
	if (data == NULL) {
		err_quit("Get_ifi return null");
	}
	
	printf("Checking if network interfaces:\n");
	
	
	while (data) {
	
		unsigned long netmask = ((struct sockaddr_in *)data->ifi_ntmaddr)->sin_addr.s_addr;
		unsigned long serverad = IPserver.s_addr;
		unsigned long localad = ((struct sockaddr_in *)data->ifi_addr)->sin_addr.s_addr;
		
		
		printf("\tName: %s, Address: %s, Network Mask: %s\n", data->ifi_name, PRINT_ADDRESS(data->ifi_addr), PRINT_ADDRESS(data->ifi_ntmaddr));
		
		if ((localad & netmask) == (serverad & netmask)) {
			local = 1;
		}
		data = data->ifi_next;
	}
	
	if (local == 1) {
		printf("Server is Local\n");
	}
	else {
		printf("Server is Not Local\n");
	}
}

// Send a packet safely through UDP
void
send_safe(int sockfd, const void *buffer, size_t len, const struct sockaddr *to) 
{
	sendto(sockfd, buffer, len, 0, to, sizeof(struct sockaddr));
}

// Start listening
void
bind_recv()
{
	recvfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (recvfd < 0) {
		err_quit("Error creating recieving end socket");
	}
	
	if (local == 1) {
		int optval = 1;
		//setsockopt(*iptr, SOL_SOCKET, SO_REUSEADDR | SO_DONTROUTE, &optval, sizeof optval);
		setsockopt(recvfd, SOL_SOCKET, SO_REUSEADDR | SO_DONTROUTE, &optval, sizeof optval);
	}
	
	memset((char *)&IPclient, 0, sizeof(IPclient));
	IPclient.sin_family = AF_INET;
	IPclient.sin_port = 0;
	IPclient.sin_addr.s_addr = htonl(INADDR_ANY);
	
	if (bind(recvfd, (struct sockaddr*)&IPclient, sizeof(IPclient)) == -1) {
		err_quit("Error binding recieving end");
	}
	
	
	struct sockaddr_in current_addr;	
	memset((char *)&current_addr, 0, sizeof(current_addr));
	socklen_t current_len = sizeof(current_addr);
	if (getsockname(recvfd, (struct sockaddr*)&current_addr, &current_len) == -1) {
		err_quit("Get Socket Name failed");
	}

	printf("IPclient bounded to: %s:%u\n", inet_ntoa(current_addr.sin_addr), current_addr.sin_port);
}

// Get the filetransfer port from server
unsigned short
get_transfer_port()
{
	// Send the filename to server
	socklen_t salen;
	struct sockaddr_in sa;
	memset((char *)&sa, 0, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_port = htons(atoi(settings_Port));
	if (inet_aton(settings_IP, &sa.sin_addr) == 0) {
		err_quit("Error parsing server IP\n");
	}
	
	send_safe(recvfd, settings_Filename, strlen(settings_Filename), (struct sockaddr*)&sa);
	printf("Sent filename, waiting for server to return transfer socket port number\n");
	
	// Get transfer socket port number from server
	size_t bytes_read;
	char buffer[512];
	bytes_read = recvfrom(recvfd, buffer, 200, 0, (struct sockaddr*)&sa, &salen);
	
	printf("Server returned %d bytes", bytes_read);
	if (bytes_read == sizeof(unsigned short)) 
	{
		unsigned short portNumber = *((unsigned short*)buffer);		
		return portNumber;
	}
	else
	{
		printf("Unexpected data from server, %d bytes. Was expecting port number.", bytes_read);
	}
	
	return 0;
}

// Start actual file transfer
void
file_transfer(unsigned short port)
{
	// Send the ack to server
	socklen_t salen;
	struct sockaddr_in sa;

	memset((char *)&sa, 0, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_port = htons(port);
	if (inet_aton(settings_IP, &sa.sin_addr) == 0) {
		err_quit("Error parsing server IP\n");
	}
		
	send_safe(recvfd, "YAY", strlen("YAY"), (struct sockaddr*)&sa);
	
	// Should start receiving now
	printf("Ready to start receiving files\n");
	
	char buffer[512];
	int filesize, bytes_read;
	memset(buffer,0,sizeof(buffer));
	
	bytes_read = recvfrom(recvfd, &filesize, 512, 0, (struct sockaddr*)&sa, &salen);
	printf("bytes_read : %d\n",bytes_read);
	if (bytes_read == sizeof(filesize)) 
	{
		//filesize = *((int*)buffer);
		
		if (filesize == -1) {
			printf("\n-- File Transfer Failed, File Not Found --\n");
			return;
		}
		else {
			printf("fileSIZE : %d \n",filesize); 
		}
	}
	else
	{
		err_quit("Unexpected response from server, was expecting filesize");
	}

	printf("SERVER CONNECTION INFO : \n");
	printf("\t SERVER PORT : %d\n\t SERVER SOCKFD : %d\n",ntohs(sa.sin_port),recvfd);

#if 0
	
	memset(buffer,0,sizeof(buffer));
	while (filesize > 0) {
		bytes_read = recvfrom(recvfd, buffer, 512, 0, (struct sockaddr*)&sa, &salen);
		//buffer[bytes_read] = 0;
		printf("%s",buffer);
		memset(buffer,0,sizeof(buffer));
		filesize = filesize - bytes_read;
	}

#endif
	printf("recvfd : %d\n",recvfd);
	init_client_wndw();
	int rc = createThreads(recvfd,(struct sockaddr *)&sa);

	if(rc == EXIT_SUCCESS)
		printf("\n-- File Complete --\n");
	else
		printf("\n-- File Transfer Failed --\n");
	
}

int
main(int argc, char **argv)
{
	if (read_settings() != 0) {
		err_quit("Error loading settings");
	}

	printf("Server: %s:%s\n", settings_IP, settings_Port);
	if (inet_aton(settings_IP, &IPserver) == 0) {
		err_quit("Error parsing server IP\n");
	}
	
	determine_locality();
	
	// Set up the recieving end
	bind_recv();
	
	// Send the filename and get transfer port
	unsigned short transfer_port = get_transfer_port();
	printf("Server returned transfer port: %u\n", transfer_port);
	
	// Start file transfer
	file_transfer(transfer_port);
	
	exit(0);
}
