/*
 ============================================================================
 Name        : CS533_Client.c
 Author      : Karthik Uthaman
 Version     : 1.0
 Copyright   : Published under GNU License. 
 Description : This is the main entry point for the client. It takes user input
		and starts the requested client by forking a child process. We 
		also create a pipe through which the child process can communicate
		any status to the parent process. 
		ERROR CONDITIONS HANDLED :	
			1. Server Crashing 
			2. Parent process crashing
			3. Child process crashing 
 Created on  : Sep 21, 2011
 ============================================================================
 */

#include "CS533_Client.h"

#define INVALID_ADDR 53301
#define INVALID_PORT 53302
#define CONNECT_FAILURE 53303
#define FORK_FAILURE 53304

#define MAX_PORT_LEN 32
#define MAX_IP_LEN 16

static sigset_t signal_mask;

/*****************FUNCTION DEFINITION****************************

DESCRIPTION : signal_thread is the main thread that will be 
used to handle signals raised in the process. we mainly use this 
to handle SIGPIPE AND SIGCHLD.

*****************************************************************/
void *signal_thread (void *arg)
{
    int       sig_caught;    /* signal caught       */
    int       rc;            /* returned code       */


    rc = sigwait (&signal_mask, &sig_caught);
    if (rc != 0) {
        /* handle error */
    }
    switch (sig_caught)
    {
#if 0
    case SIGINT:     /* process SIGINT  */
	printf("SIGINT RECVD\n");
        break;
    case SIGTERM:    /* process SIGTERM */
	printf("SIGTERM RECVD\n");
        break;
#endif
    case SIGPIPE:
	printf("\nSTATUS : SIGPIPE RECVD\n");
        break;
#if 0
    case SIGCHLD:
	printf("\nSTATUS : SIGCHLD RECVD\n");
	break;
#endif
    default:        /* should normally not happen */
        fprintf (stderr, "\nUnexpected signal %d\n", sig_caught);
        break;
    }
}


/*****************FUNCTION DEFINITION***********************************

Description : Main entry point to the client process. Creates the sockets
and calls the startClient function. 

************************************************************************/


int main(int argc, char* argv[]) {

	struct hostent hptr;
	int isServerValid = false, isPortValid = true;
	int errorcode = EXIT_SUCCESS;
	int rc;

	//Check for the no.of cmd line args...	
	if(argc!=2)
	{
		printf("usage : ./CS553_Client <ip/hostname> \n");
		errorcode = EXIT_FAILURE;
	}
	else
	{

#if 0
		//add the signals you process needs to handle to the signal set struct.
	        sigemptyset (&signal_mask);
        	//sigaddset (&signal_mask, SIGINT);
        	//sigaddset (&signal_mask, SIGTERM);
		sigaddset(&signal_mask,SIGCHLD);
        	sigaddset (&signal_mask, SIGPIPE);

		//Create the signal thread as the first thread so that any thread created
		//after this will inherit the same signal handling features.

        	rc = pthread_sigmask (SIG_BLOCK, &signal_mask, NULL);

        	if (rc != 0) {
         	/* handle error */
        	}
        	int sig_thr_id;

        	rc = pthread_create (&sig_thr_id, NULL, signal_thread, NULL);
#endif

		

		//Start the client process...
	
		startClient(argv[1]);
	}

	return errorcode;
}

void startClient(char *hostaddr)
{
	/* Start the client
	 * 1. Create socket and connect to the server.  	--> NOT DONE
	 * 2. Find the services available with the server. 	--> NOT DONE
	 * 4. Create sockets
	 * 4. List the services
	 * 5. Take input and fork a child with xterm
	 */

	struct sockaddr_in serv_addr;
	int errcode ;
	int isConnected_echo = false;
	int isConnected_daytime = false;
	int choice = 0;
	char port[MAX_PORT_LEN];
	char ip_addr[MAX_IP_LEN];
	char serv_port[10];
	int echo_serv_sockfd;
	int daytime_serv_sockfd;

	//using validation of ip or host-name is handled inside Getsockaddr. 

        if(Getsockaddr(hostaddr,&serv_addr) < 0)
	{
		printf("ERROR : Cannot initialize client...\n");
		exit(-1);
	}

        char ipstr[20];
        memset(ipstr,0,sizeof(ipstr));
        inet_ntop(AF_INET, &(serv_addr.sin_addr), ipstr, sizeof(ipstr));



	//Establish a test connection to see server is accepting connection...
	memset(serv_port,0,sizeof(serv_port));
	sprintf(serv_port,"%d",ECHO_PORT);

	memset(serv_port,0,sizeof(serv_port));
	
	sprintf(serv_port,"%d",ECHO_PORT);


	int pfd[2];

	while(1)
	{


	if(serv_port && ipstr)
	{

		close(echo_serv_sockfd);
		close(daytime_serv_sockfd);
		
		printf("\n------------------------------------------------------------------------------\n");

		printf("Services : \n\t1. ECHO CLIENT\n\t2. DAY TIME CLIENT\n\t3. QUIT \nPlease Enter a Service number : ");
		scanf("%d",&choice);

		memset(ip_addr,0,sizeof(ip_addr));
		strcpy(ip_addr,inet_ntoa(serv_addr.sin_addr));

		
		switch(choice)
		{
			case 1:
				memset(serv_port,0,sizeof(serv_port));
				sprintf(serv_port,"%d",ECHO_PORT);
				invoke_child("/home/stufs1/kuthaman/CS533_Assignment1/bin/echocli",ip_addr,serv_port);
				break;
			case 2:
				memset(serv_port,0,sizeof(serv_port));
				sprintf(serv_port,"%d",DAYTIME_PORT);
				invoke_child("/home/stufs1/kuthaman/CS533_Assignment1/bin/daytimecli",ip_addr,serv_port);
				break;
			case 3:
				exit(0);
				
			default:
				printf("INVALID INPUT :  TRY AGAIN ... \n");
				break;
		}
	}
	else
	{
		err_msg(errno,strerror(errno));
		errcode = CONNECT_FAILURE;
		break;
	}
	}
}

void invoke_child(char *client_name, char *ip_addr, char *port)
{

	int pfd[2];
	int pid;
	int errcode;

	char buff[1024];
	char pfd_str[20];
	int status=0;


	/*
	* Create a communication pipe that can be used by child to write status back to parent.
	*/
	pipe(pfd);


	/*
	* create a child process and load the requested client process with xterm. 
	* on the parent side, read the pipe for any status msg and on the death of child process 
	* ask the user to select another service to connect to...
	*/

	pid = fork();

	switch (pid)
	{
		case -1:
			err_msg(errno,strerror(errno));
			errcode=FORK_FAILURE;
			break;
		case 0:
			//CHILD PROCESS
			close(pfd[0]);
			memset(pfd_str,0,sizeof(pfd_str));
			sprintf(pfd_str,"%d",pfd[1]);
			//printf("%s %s %s %s\n",client_name,ip_addr,port,pfd_str);
			execlp("xterm", "xterm", "-e",client_name,ip_addr,port,pfd_str, (char *) 0);
			//execlp("xterm", "xterm", "-e", "/bin/sh", "-c",client_name,ip_addr,port,pfd_str, (char *) 0);
			break;
		default:
			close(pfd[1]);
			memset(buff,0,sizeof(buff));

			#if 1
			while(read(pfd[0],buff,sizeof(buff)))
			{
				puts(buff);
				memset(buff,0,sizeof(buff));
			}
			#endif
			wait(&status);
			//Child exits;
			//handle error;
	}

}
