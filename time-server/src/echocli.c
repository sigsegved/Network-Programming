/*
 ============================================================================
 Name        : echocli.c
 Author      : Karthik Uthaman
 Version     :
 Copyright   : Published under GNU License.
 Description :
 Created on	 : Sep 22, 2011
 ============================================================================
 */

#include "utils.h"
#include "cs533.h"


int main(int argc, char *argv[])
{

	struct sockaddr_in serv_addr;
	int port;
	struct hostent hptr;
	int serv_sockfd;
	int isConnected,pfd,rc;
	char buff[1024],msg[1024];

	if(argc!=4)
	{
		fprintf(stderr,"ERROR \t: USAGE : echocli <ip/host-name> <port> <pipe-fd>\n");
		return EXIT_FAILURE;
	}

	pfd = atoi(argv[3]);


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

	//serv_sockfd = createtcpsocket(port,&serv_addr);
        serv_addr.sin_port = htons(port);
        serv_sockfd = Socket(AF_INET, SOCK_STREAM, 0);


	if(serv_sockfd > 0 )
		isConnected = tcpConnect(serv_sockfd,serv_addr,port);



	char status[1024];

	int errcode = EXIT_SUCCESS;

	memset(status,0,sizeof(status));

	if(isConnected)
	{
		strcpy(status,"STATUS \t: Connected to Server...");
		//write_pipe(pfd,status,strlen(status));
	}
	else
	{
		strcpy(status,"STATUS \t: ");
		strcat(status,strerror(errno));
		//printf("%s\n",status);
		write_pipe(pfd,status,strlen(status));
	}
		
	



	int l=0;
        struct timeval       timeout;
        fd_set        master_fds, curr_fds;
        int maxfd;
        int ready_fd_count =0;

	int i=0;
	printf("-----------------------------------------------------------------------\n ECHO MSG \t: ");
	char back_buff[1024];
	memset(back_buff,0,sizeof(back_buff));

        do
        {
		FD_ZERO(&master_fds);
		maxfd = serv_sockfd+1;
		FD_SET(fileno(stdin),&master_fds);
		FD_SET(serv_sockfd,&master_fds);
		
		fflush(stdout);
		fflush(stdout);
		memset(msg,0,sizeof(msg));
		memset(buff,0,sizeof(buff));
		
		rc = select(maxfd,&master_fds,NULL,NULL,NULL);
		
		if(rc < 0)
		{
			perror("select() failed");
			break;
		}
		if( rc == 0)
		{
			printf("SELECT TIMED OUT\n");
			break;
		}

		for(i=0;i<maxfd;i++)
		{

		    if(FD_ISSET(i,&master_fds))
		    {
			if(i==fileno(stdin))
			{
				memset(msg,0,sizeof(msg));
				fgets(msg, sizeof(msg),stdin);
				strcpy(back_buff,msg);
				
				l = strlen(msg);
				msg[l] = '\n';
				if((l=writen(serv_sockfd,msg,l)) <0)
				{
					//send to parent that the echo request was sent to server.
					memset(status,0,sizeof(status));
					strcpy(status,"ERROR \t: Write to Echo Server Failed");
					//printf("%s\n",status);

					isConnected = false;
					shutdown(serv_sockfd,2);
					close(serv_sockfd);
			
					rc = write_pipe(pfd,status,sizeof(buff));
					if(rc == -1)
						printf("PIPE CLOSED. I could be an orphan process now.. :(");
			
					break;
				}
			}
			if(i==serv_sockfd)
			{
				memset(buff,0,sizeof(buff));
				if((rc=read(serv_sockfd,buff,sizeof(buff)))<0)
				{
					//send to parent that echo server responded back.
					memset(status,0,sizeof(status));
					strcpy(status,"ERROR \t: Read from Echo Server Failed");
					//rc = write_pipe(pfd,status,sizeof(status));
					shutdown(serv_sockfd,2);
					close(serv_sockfd);
					isConnected = false;
					break;
					//puts(buff);
	
				}
				else
				{
					//printf("LOG : read %d bytes from server \n",rc);
					if(rc == 0)
					{
						if(errno == EINTR)
						{
							printf("STATUS \t: EINTR\n");
							continue;
						}
						else
						{
							isConnected = false;
							memset(status,0,sizeof(status));
							strcpy(status,"STATUS \t: ECHO SERVER IS DOWN");
							rc = write_pipe(pfd,status,strlen(status));
							
							break;
						}
					}
					printf("Echo Response \t: %s",buff);
					printf("-----------------------------------------------------------------------\nECHO MSG \t: ");
					fflush(stdout);
					fflush(stdout);
					memset(status,0,sizeof(status));
	
					if(strcmp(back_buff,buff) == 0)
					{
						strcpy(status,"STATUS \t: ECHO SERVER WORKS PROPERLY");
						rc = write_pipe(pfd,status,strlen(status));
						//printf("%s\n",status);
					}
					else
					{
						strcpy(status,"STATUS \t: ECHO SERVER IS ERRORNEOUS");
						rc = write_pipe(pfd,status,strlen(status));
						//printf("%s\n",status);
						//printf("SENT : %s\n RECVD :%s\n",back_buff,buff);
						errcode = EXIT_FAILURE;
						break;
					}
					memset(back_buff,0,sizeof(back_buff));
				}
			}
		    }
		}




	}while(isConnected);


	printf("I am done\n");

	shutdown(serv_sockfd,2);
	close(serv_sockfd);

	exit(-1);

}

