/*
 ============================================================================
 Name        : echotimecli.c
 Author      : Karthik Uthaman
 Version     :
 Copyright   : Published under GNU License.
 Description :
 Created on	 : Sep 22, 2011
 ============================================================================
 */

#include "echotimecli.h"
#include "utils.h"
int main(int argc, char *argv[])
{

	struct sockaddr_in serv_addr;
	int port;
	struct hostent *hptr;
	int serv_sockfd;
	int isConnected;
	int rc,pfd;
	char msg[1024];
	char buff[1024];

        if(argc!=4)
        {
                fprintf(stderr,"ERROR \t: USAGE : echocli <ip/host-name> <port> \n");
		sleep(5);
                return EXIT_FAILURE;
        }

        Getsockaddr(argv[1],&serv_addr);
        char ipstr[20];
        memset(ipstr,0,sizeof(ipstr));
        inet_ntop(AF_INET, &(serv_addr.sin_addr), ipstr, sizeof(ipstr));

        port = atoi(argv[2]);
        if(port < 1025)
        {
                fprintf(stderr,"ERROR \t: PORT NUMBER SHOULD BE GREATER THAN 1024\n");
                return EXIT_FAILURE;
        }

        serv_addr.sin_port = htons(port);
        serv_sockfd = Socket(AF_INET, SOCK_STREAM, 0);


        if(serv_sockfd > 0 )
		isConnected = tcpConnect(serv_sockfd,serv_addr,port);


	char status[1024];
	char errbuff[1024];
	int errcode = EXIT_SUCCESS;

	memset(status,0,sizeof(status));
	pfd = atoi(argv[3]);

        if(isConnected)
        {
                strcpy(status,"STATUS \t: Connected to Server...");
                write_pipe(pfd,status,strlen(status));
        }
        else
        {
                strcpy(status,"STATUS \t: ");
                strcat(status,strerror(errno));
                write_pipe(pfd,status,strlen(status));
        }


	

	
	memset(status,0,sizeof(buff));
	memset(errbuff,0,sizeof(errbuff));
	strcpy(status,"STATUS : Day time server responded");
	strcpy(errbuff,"STATUS : Day time server did not reposnd");


        struct timeval       timeout;
        fd_set        master_fds, curr_fds;
        int maxfd;




	while(isConnected)
	{
        	FD_ZERO(&master_fds);
       		maxfd = serv_sockfd;
        	FD_SET(serv_sockfd, &master_fds);
        	memcpy(&curr_fds, &master_fds, sizeof(master_fds));
		memset(buff,0,sizeof(buff));


                rc = select(maxfd + 1, &curr_fds, NULL, NULL, NULL);
                if (rc < 0)
                {
 	               perror("select() failed");
                       break;
                }
                if (rc == 0)
                {
               		printf("select() timed out.  End program.\n");
                        break;
                }



		if((rc=Readline(serv_sockfd,buff,sizeof(buff))) <=0)
		{
			//send to parent that echo server responded back.
			rc = write_pipe(pfd,errbuff,sizeof(buff));

                	if(rc == 0)
			{
					printf("STATUS \t: EINTR\n");
					continue;
			}
			else
			{
					isConnected = false;
					memset(status,0,sizeof(status));
					strcpy(status,"STATUS \t: DAYTIME SERVER IS DOWN");
					rc = write_pipe(pfd,status,strlen(status));
					break;
			}


		}
		else
		{
			printf("Server Time  \t: %s",buff);
			rc = write_pipe(pfd,status,sizeof(status));
		}


	}

	shutdown(serv_sockfd,2);
	close(serv_sockfd);
	return errcode;
}

