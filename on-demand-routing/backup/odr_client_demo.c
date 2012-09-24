#include "odr_client.h"

int main()
{
	odr_client_com data;
	printf("Binding server...\n");
	int sock = server_bind();
	
	while (1) {
		printf("Waiting for event...\n");
		server_getevent(sock, &data);
		
		if (data.event == EVENT_SEND)
		{
			printf("Request to send '%s' to '%s:%d'\n", data.msg, data.ip, data.dst_port);
			
			// Sending to self
			server_sendevent(sock, data.ip, data.src_port, data.dst_port, data.msg);
		}
		
		printf("Event raised!\n");
	}
	
	printf("buh bye\n");
}
