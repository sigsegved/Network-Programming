/*
 * superserver.c
 *
 *  Created on: Sep 22, 2011
 *      Author: ukay
 */

#include "CS533_Server.h"


#define MAX_SERVICES 2
#define ECHOS_FAILURE 1
#define DAYTIME_FAILURE 2
#define ALL_SERVICE_FAILURE 3

void *signal_thread (void *arg);

//the signal mask!!!

static sigset_t   signal_mask;


int global_maxfd;

int main (int argc, char *argv[])
{
	int echoSockFD,daytimeSockFD;
	int rc;
/*
 * Create the signal masks and start the signal_handler thread
 */

	sigemptyset (&signal_mask);
    	sigaddset (&signal_mask, SIGINT);
    	sigaddset (&signal_mask, SIGTERM);
    	sigaddset (&signal_mask, SIGPIPE);
    	rc = pthread_sigmask (SIG_BLOCK, &signal_mask, NULL);

    	if (rc != 0) {
       	 /* handle error */
    	}
	int sig_thr_id;

	rc = pthread_create (&sig_thr_id, NULL, signal_thread, NULL);
    	if (rc != 0) {
    	}

/*
 * startServices will create the sockets and put them in listen mode and call the superServer()
 */

	if(rc=startServices(&echoSockFD,&daytimeSockFD) == NO_ERROR)
	{
		printf("LOG \t: Server Initialized Successfully...\n");
		printf("LOG \t: Starting the super server...\n");
		startSuperServer(echoSockFD,daytimeSockFD);
	}
	else
		printf("ERROR \t: Server failed to start\n");
}

/* Signal handler code */
void *signal_thread (void *arg)
{
    int       sig_caught;    /* signal caught       */
    int       rc;            /* returned code       */

	int i;

    rc = sigwait (&signal_mask,&sig_caught);
    if (rc != 0) {
        /* handle error */
    }
    switch (sig_caught)
    {
    case SIGINT:     /* process SIGINT  */
	printf("LOG \t: CLEANING UP SERVER BEFORE EXITING...\n");
	for(i=3; i<=global_maxfd;i++)
	{
		shutdown(i,2);
		close(i);
	}
	printf("LOG \t: MAY DAY!!! MAY DAY!!!\nLOG \t: SERVER GOING DOWN\n-------------------THE END------------------------\n");
	exit(-1);
        break;
    case SIGTERM:    /* process SIGTERM */
	printf("SIG TERM RECVD");
        break;
    case SIGPIPE:
	break;
    default:         /* should normally not happen */
        fprintf (stderr, "\nUnexpected signal %d\n", sig_caught);
        break;
    }
}


/*
 * Acts like the inetd daemon for the services here. 
 * Accepts an incoming connection and calls the particular thread to handle the same.
 */

int startSuperServer(int echoSockFD,int daytimeSockFD)
{
	struct timeval       timeout;
	fd_set        master_fds, curr_fds;
	int maxfd,rc;
	int ready_fd_count =0;


	do
	{
		FD_ZERO(&master_fds);
		FD_ZERO(&curr_fds);
		maxfd = (echoSockFD > daytimeSockFD)?echoSockFD:daytimeSockFD;
		maxfd = maxfd+1;
		FD_SET(echoSockFD, &master_fds);
		FD_SET(daytimeSockFD, &master_fds);

		memcpy(&curr_fds, &master_fds, sizeof(master_fds));

		printf("LOG \t: Waiting on select()...\n");

		rc = select(maxfd , &curr_fds, NULL, NULL, NULL);

		if (rc < 0)
		{
			perror("  select() failed");
			break;
		}
		if (rc == 0)
		{
			printf("  select() timed out.  End program.\n");
			break;
		}

		ready_fd_count = rc;
		int i=0;

		for( i = 0 ; i < maxfd+1 ; i++)
		{
			if(FD_ISSET(i,&curr_fds))
			{
				if( i == echoSockFD || i==daytimeSockFD)
					acceptConnections(i,echoSockFD,daytimeSockFD);
				ready_fd_count--;

			}
		}

	}while(1);

}

void acceptConnections(int serviceID, int echoSockFD, int daytimeSockFD)
{
	char client_type[128];
	int client_fd;
	int ret_val;
	int thread_id;
	//do
	{
		client_fd = accept(serviceID, NULL, NULL);
		if (client_fd < 0 )
		{
			//currently of no use as the sockets are not non_blocking.
			if(errno == EWOULDBLOCK)		//finished accepting all connection.
				perror("ERROR \t:");
				//break;
			else
				printf("LOG \t: Error in accepting connection\n");
		}
		else
		{
			global_maxfd = client_fd;
			if(serviceID == echoSockFD)
			{
				ret_val = pthread_create(&thread_id, NULL,echoService, (void *) client_fd);
				memset(client_type,0,sizeof(client_type));
				strcpy(client_type,"Echo Client");
			}
			else
			{
				ret_val = pthread_create(&thread_id, NULL, daytimeservice, (void *) client_fd);
				memset(client_type,0,sizeof(client_type));
				strcpy(client_type,"Daytime Client");
			}
			printf("LOG \t: A new thread has been created to handle a %s\n",client_type);
		}
	}
	//while(client_fd!=-1);		//to avoid lingering here as the sockets are not non-blocking.
}

int startServices(int *echoFD, int *dtFD)
{
	int echoSockFD;
	int daytimeSockFD;
	int rc,rc1;

	rc = createServerSocket(ECHO_PORT,&echoSockFD) ;
	rc1 = createServerSocket(DAYTIME_PORT,&daytimeSockFD) ;

	rc = rc + rc1;

	*echoFD =  echoSockFD;
	*dtFD = daytimeSockFD;
	return rc;
}

int createServerSocket(int port,int *sockfd)
{
	int rc=NO_ERROR;
	int listenSockFD;
	struct sockaddr_in servaddr;
	int yes=1;
	int backlog;

	listenSockFD = socket(AF_INET, SOCK_STREAM, 0);
	bzero(&servaddr, sizeof (servaddr));
    	int on = 1;
    	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = ntohl(INADDR_ANY);
	servaddr.sin_port = ntohs(port); /* daytime server */

	if (setsockopt(listenSockFD, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof (int)) == -1) {
		perror("setsockopt");
		rc = FATAL_ERROR;
	}

#if 0
	rc = fcntl(listenSockFD, F_SETFL, O_NONBLOCK);
	if (rc < 0)
	{
	      perror("ioctl() failed");
	      close(listenSockFD);
	      //exit(-1);
	}
#endif
	else
	{
		if ((bind(listenSockFD, (struct sockaddr *) & servaddr, sizeof (servaddr))) < 0) {
				//handle error condition...]
				perror("bind:");
				rc = BIND_ERROR;
		}
		else
		{
			if ((rc = listen(listenSockFD, backlog)) < 0) {
				perror("listen");
				rc = FATAL_ERROR;
			}
			else
				printf("LOG \t: LISTENING in PORT : %d\n",port);
		}
	}
	*sockfd = listenSockFD;
    return rc;
}

