#include "odr_client.h"
#include "time.h"

int main()
{
	int sock = client_init(123);
	time_t ticks;
 	
	while (1) {
		char sourceip[20];
		int port = 0;
		char buff[1024];
		printf("Waiting for requests..\n");
		int ret = msg_recv(sock, sourceip, &port, buff);
		
		if (ret == RECV_OK) {
			printf("Request: %s %d: %s, sending back\n", sourceip, port, buff);

			ticks = time(NULL);
			snprintf(buff, sizeof(buff), "%.24s\r\n", ctime(&ticks));
			
			msg_send(sock, sourceip, port, buff, 0);
		}
		else if (ret == RECV_ERROR) {
			break;
		}
	}
	
	printf("buh bye");
	return 0;
}
