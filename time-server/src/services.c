/*
 * services.c
 *
 *  Created on: Sep 24, 2011
 *      Author: ukay
 */

#include "services.h"

#define MAX_BREAK_WAIT 100

void echoService(void *sockFD)
{

	//pthread_mutex_lock();

	int cli_sockfd = (int)sockFD;
	char buff[MAX_BUFF_LEN];
	int bytes;
	int rc=0;
	int maxfdp;
	int break_wait =0;

	int isConnected = 1;

	//printf("CLIENT SOCK FD : %d\n\n",cli_sockfd);
	fd_set rset;

	do
	{

		FD_ZERO(&rset);
		FD_SET(cli_sockfd,&rset);
		maxfdp=cli_sockfd+1;


		rc = select(maxfdp,&rset,NULL,NULL,NULL);
		//printf("LOG \t: OUT OF SELECT :%d\n",rc);
		

		if(rc ==0)
		{
			printf("TIMEOUT\n");
		}	
		if(rc == -1)
		{
			printf("SELEECT ERROR\n");
		}


		if(FD_ISSET(cli_sockfd,&rset))
		{
			while((bytes = Readline(cli_sockfd,buff,sizeof(buff))) >= 0)
			{
			if(bytes>0)
			{
				printf("LOG \t: Bytes read from Client : %d\n",bytes);
				printf("LOG \t: Data from Client : %s",buff);
				bytes = writen(cli_sockfd,buff,bytes);
				printf("LOG \t: Number of bytes written back to client : %d\n",bytes);
				memset(buff,0,sizeof(buff));
				bytes = 0;
			}
			else if((bytes == 0) && (break_wait < MAX_BREAK_WAIT))
			{

				if(errno ==  EINTR)
				{
					break_wait++;
					printf("WARNING \t:BREAK WAITING\n");
				}
				else
				{
					printf("LOG \t: Echo Client Terminated\n");
					isConnected = 0;
					break;
				}
				

			}
			else
			{
				isConnected = 0;
				break;
			}
			}
			if( bytes < 0)
				isConnected = false;
		}
	}while(isConnected);


	printf("LOG \t: Echo Thread is terminated \n");
	shutdown(cli_sockfd,2);
	close(cli_sockfd);

	//pthread_mutex_unlock();


}

void daytimeservice(void *sockfd)
{
	int cli_sockfd = (int)sockfd;
	int bytes;
	char buff[1024];

	struct timeval tout;
	fd_set rset;

	tout.tv_sec = 1;
	tout.tv_usec = 0;
	int client_alive = true;
	int maxfd;
	int rc;

	do
	{
		FD_ZERO(&rset);
		FD_SET(cli_sockfd,&rset);	
		maxfd = cli_sockfd+1;

		rc = select(maxfd,&rset,NULL,NULL,&tout);

		if(rc == 0 )
		{
		memset(buff,0,sizeof(buff));
		getCurrTime(buff);
		if((bytes=writen(cli_sockfd,buff,strlen(buff))) > 0)
			client_alive = true;
		else
		{
			if(errno == EPIPE)
				printf("LOG \t: Client is no more alive\n");

			client_alive = false;
		}
		}
		else if(rc < 0)
		{
			printf("LOG \t: Select Error\n");
			client_alive = 0;
			break;
		}
		else
		{
			printf("LOG \t: Client Disconnected\n");
			client_alive = 0;
			break;
		}
		
		

	}while(client_alive);
	printf("LOG \t: DayTime Thread is terminated \n");
	shutdown(cli_sockfd,2);
	close(cli_sockfd);


}

void getCurrTime(char *buff)
{
	time_t ticks;
    	ticks = time(NULL);
	strcpy(buff,ctime(&ticks));	
	//int l = strlen(buff);
	//buff[l]='\n';
    	//snprintf(buff, sizeof(buff), "%.24s\r\n", ctime(&ticks));
	//puts(buff);
}
