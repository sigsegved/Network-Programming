#include "CS533.h"
#include "letsdothis.h"
//Note to self
//port: 46578 - echo
//port: 46577 - time

struct sock_struct
{
  int fd;
  struct sockaddr *ipaddr;
  struct sockaddr *netmask;
  struct sockaddr *netid;
};

static void SIGCHLD_HANDLER(int signo) //handler for when child processes terminate
{
  pid_t pid;
  int stat;

  while((pid = waitpid(-1, &stat, WNOHANG)) > 0) //reap exit status
    write(1, "Server: Child terminated\n", 25);
  return;
}
		    
void ChildHandler(int sockfd, struct sockaddr_in client, char filename[4096])
{
  pid_t me;
  FILE *Outfile;
  char buffer[512];
  int bytesread;
  int filelen;
  int cli_len;
  const int on = 1;

  me = getpid();
 
  cli_len = sizeof(client);
  printf("Child %d: Starting transfer\n", me);
  
  printf("Child %d: Opening file: %s\n", me, filename);
  Outfile = fopen(filename, "r");
  if(Outfile == NULL)
    {
      perror("Child Error: fopen");
      exit(1);
    }

  fseek(Outfile, 0, SEEK_END);
  filelen = ftell(Outfile);
  rewind(Outfile);
  printf("Child %d:   File length: %d\n", me, filelen);

 
  printf("CLIENT INFO  : \n\t CLIENT PORT  : %d \n\t CLIENT SOCKFD : %d\n",ntohs(client.sin_port),sockfd);
  sendto(sockfd, &filelen, sizeof(filelen), 0, (struct sockaddr *)&client, sizeof(client));
  peerInfo peer;
  	memset(&peer,0,sizeof(peer));
	peer.sockfd = sockfd;
	memcpy(&peer.saddr,&client,sizeof(struct sockaddr));
  init_server_window(20); 
  int bytes_to_read  = 512 - sizeof(tcpHeader);
  int last_packet = 0;
  while(!feof(Outfile) && Outfile)
  {
      bytesread = fread(buffer, 1,  bytes_to_read, Outfile);
      //printf("Child %d: Sending: %s\n", me, buffer);
      //printf("Child %d: %d Bytes\n", me, bytesread);
      //sendto(newsockfd, buffer, bytesread, 0, (struct sockaddr *)client, sizeof(*client));

      //printf("ASKING reliableSendUDP to send : %d Bytes\n",bytesread);

      last_packet = feof(Outfile);


      reliableSendUDP(peer,buffer,bytesread,last_packet);
	
  }
  printf("Child %d: Transfer complete! Closing connection.\n", me);
  fclose(Outfile);
  close(sockfd);
  exit(0);
}

int main(int argc, char *argv[])
{
  FILE *serv_in;
  const int on = 1;
  int port = 0, windowsize = 0, i = 0, maxfd = 0, bytesread = 0, temp = 0;
  int  ifi_count = 0, option = 0;
  int newsockfd = 0;
  struct ifi_info *ifi, *ifihead;
  struct sockaddr_in newsock;
  struct sockaddr_in cliaddr;
  struct sockaddr_in testaddr;
  struct sockaddr *sa;
  struct sock_struct *addr_list;
  unsigned int newsock_len = 0;
  socklen_t cliaddr_size;
  fd_set rset;
  char buffer[4096], client[16];
  char filename[4096];
  u_char *ptr;
  pid_t pid;
  struct sigaction act;

  memset(&act, 0, sizeof(act));

  act.sa_handler = SIGCHLD_HANDLER; //initializes the signal handler struct
  sigemptyset(&act.sa_mask);

  if (sigaction(SIGCHLD, &act, 0)) //sets signal action
    {
      perror ("sigaction");
      exit(1);
    }

  printf("Server: Starting server\n");
  
  FD_ZERO(&rset); 
  
  printf("Server: Reading server.in\n");
  
  /*READ server.in*/
  serv_in = fopen("server.in", "r");
  if(serv_in == NULL)
    {
      perror("Server:");
      exit(1);
    }
  while(fgets(buffer, 4096, serv_in) != NULL)
    {
      if(port == 0) //first line read
	{
	  port = atoi(buffer);
	  if(port <= 0) //sanity check the port number
	    {
	      printf("Server: Invalid port number\n");
	      exit(1);
	    }
	}
      else
	{
	  if(windowsize == 0) //second line read
	    {
	      windowsize = atoi(buffer);
	      if(windowsize < 0)
		{
		  printf("Server: Invalid window size\n");
		  exit(1);
		}
	    }
	}
    }
  printf("Port: %d\n", port);
  printf("Window size: %d\n", windowsize);
  
  /*BEGIN Init*/
  //Determine interface info
  ifihead = Get_ifi_info_plus(AF_INET,0);
  
  if(ifihead == NULL)
    {
      printf("Error getting interface information\n");
      exit(1);
    }

  for(ifi = ifihead; ifi != NULL; ifi = ifi->ifi_next)
    {
      ifi_count++; //counts interface entries
    }
  
  addr_list = (struct sock_struct *) calloc(ifi_count, sizeof(struct sock_struct));
  ifi = ifihead;
  printf("Server: Creating %d sockets...\n", ifi_count);
  for(i = 0; i < ifi_count; ++i) //for each entry, set up a socket and save the ipaddress
    {
      addr_list[i].fd = socket(AF_INET, SOCK_DGRAM, 0);
      
      if(addr_list[i].fd < 0)
	{
	  perror("Server: Socket");
	  free_ifi_info_plus(ifihead);
	  free(addr_list);
	  exit(1);
	}
      
      if(setsockopt(addr_list[i].fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1) //turn on addr reuse
	{
	  perror("Server: setsockopt");
	  free_ifi_info_plus(ifihead);
	  free(addr_list);
	  exit(1);
	}
      
      printf("Server:   FD = %d\n", addr_list[i].fd);
      if(addr_list[i].fd > maxfd)
	maxfd = addr_list[i].fd;
      addr_list[i].ipaddr = ifi->ifi_addr;
      addr_list[i].netmask = ifi->ifi_ntmaddr;
      addr_list[i].netid = ifi->ifi_netid;

      if ( addr_list[i].ipaddr != NULL)
	printf("Server:   IP addr: %s\n", Sock_ntop_host(addr_list[i].ipaddr, sizeof(*addr_list[i].ipaddr)));
      if ( addr_list[i].netid != NULL)
	printf("Server:   networkd id: %s\n", Sock_ntop_host(addr_list[i].netid, sizeof(*addr_list[i].netid)));
      if ( addr_list[i].netmask != NULL)
	printf("Server:   network mask: %s\n", Sock_ntop_host(addr_list[i].netmask, sizeof(*addr_list[i].netmask)));

      fcntl(addr_list[i].fd, F_SETFL, O_NONBLOCK);
      
      ((struct sockaddr_in *)addr_list[i].ipaddr)->sin_port = htons(port);
      if(bind(addr_list[i].fd, addr_list[i].ipaddr, sizeof(*(addr_list[i].ipaddr))) == -1)
	{
	  perror("Server: Bind");
	  free_ifi_info_plus(ifihead);
	  free(addr_list);
	  exit(1);
	}
      port = 0;
      port = ntohs(((struct sockaddr_in *)addr_list[i].ipaddr)->sin_port);
      printf("Server:   Port = %d\n", port);
      FD_SET(addr_list[i].fd, &rset);
      
      ifi = ifi->ifi_next;
    }
  printf("Server: ...done\nServer: Listening to sockets...\n");
  /*END Init*/

  /*Listen loop*/
  for(;;)
    {
      printf("Server: Waiting for connection...\n");
      select(maxfd + 1, &rset, NULL, NULL, NULL);
      printf("Server: Checking FDs...\n");
      
      cliaddr_size = sizeof(cliaddr);
      for(i = 0; i < ifi_count; ++i)
	{
	  if(FD_ISSET(addr_list[i].fd, &rset))
	    {
	      memset(buffer,0,sizeof(buffer));
	      bytesread = recvfrom(addr_list[i].fd, filename, 4096, 0, (struct sockaddr *) &cliaddr, &cliaddr_size);
	      inet_ntop(AF_INET, (struct sockaddr *)&(cliaddr.sin_addr), client, 16);
	      if(bytesread > 0)
		{
		  
		  printf("Server: FTP service connection accepted - %s\n", client);
		  printf("Server: client port - %d\n", ntohs(cliaddr.sin_port));
		  
		  memset(&testaddr, 0, sizeof(testaddr));
		  testaddr.sin_family = AF_INET;
		  inet_pton(AF_INET, "127.0.0.1", &testaddr.sin_addr);
		  
		  printf("Server: Determining locality...\n");
		  if(testaddr.sin_addr.s_addr == cliaddr.sin_addr.s_addr)
		    {
		      printf("Server: Client connected via loopback\n");
		      option = 0;
		    }
		  else
		    {
		      testaddr.sin_addr.s_addr = cliaddr.sin_addr.s_addr & ((struct sockaddr_in *)addr_list[i].netmask)->sin_addr.s_addr;
		      if(((struct sockaddr_in *)addr_list[i].netid)->sin_addr.s_addr == testaddr.sin_addr.s_addr)
			{
			  
			  printf("Server: Client is local\n");
			  option = 1;
			}
		      else
			{
			  printf("Server: Client is NOT local\n");
			  option = 2;
			}
		    }
		  
		  printf("Server: Creating send socket\n");
		  newsockfd = socket(AF_INET, SOCK_DGRAM, 0);
		  if(newsockfd < 0)
		    {
		      perror("Server: Send socket error");
		      exit(1);
		    }
		  
		  printf("Server: Setting socket options\n");
		  if(option == 2) //client is NOT local
		    {
		      if(setsockopt(newsockfd, SOL_SOCKET, SO_REUSEADDR | SO_DONTROUTE, &on, sizeof(on)) == -1) //turn on addr reuse
			{
			  perror("Server: setsockopt");
			  exit(1);
			}
		    }
		  else
		    {
		      if(setsockopt(newsockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1) //turn on addr reuse
			{
			  perror("Server: setsockopt");
			  exit(1);
			}
		    }
		  memcpy(&newsock, addr_list[i].ipaddr, sizeof(newsock));
		  //newsock = (struct sockaddr_in)*addr_list[i].ipaddr;
		  newsock.sin_port = htons(0);
		  
		  printf("Server: Binding listening address\n");
		  if(bind(newsockfd, (struct sockaddr *)&newsock, sizeof(newsock)) == -1)
		    {
		      perror("Server: Bind");
		      exit(1);
		    }
		  
		  newsock_len = sizeof(newsock);
		  
		  getsockname(newsockfd, (struct sockaddr *)&newsock, &newsock_len);
		  printf("Server:   Ip Address: %s\n", Sock_ntop_host((struct sockaddr *)&newsock, newsock_len));
		  printf("Server:   Port = %d\n", ntohs(newsock.sin_port));
		  printf("Server: Transmitting portnumber\n");
		  sendto(addr_list[i].fd, &ntohs(newsock.sin_port), sizeof(unsigned short), 0, (struct sockaddr *) &cliaddr, sizeof(cliaddr));
		  
		  memset(buffer,0,sizeof(buffer));
		  bytesread = recvfrom(newsockfd, buffer, 4096, 0, (struct sockaddr *) &cliaddr, &cliaddr_size);
		  printf("Server: %s received.\n", buffer);
		  
		  printf("Server: forking child\n");
		  if((pid = fork()) == 0) //if child thread.
		    {
		      printf("Child %d: Forked...\n", getpid());
		      printf("Child %d: Closing FDs\n", getpid()); 
		      for(temp = 0; temp < ifi_count; ++temp) //close all other FD's
			{
			  close(addr_list[temp].fd);
			}		  
		      
		      printf("Child %d: Entering ChildHandler\n", getpid()); 
		      ChildHandler(newsockfd, cliaddr, filename);
		    }
		}
	    }
	  //read(addr_list[i].fd, buffer, 4096);
	  FD_SET(addr_list[i].fd, &rset);
	}
    }
  
  /*CLEAN UP*/
  free_ifi_info_plus(ifihead);
  free(addr_list);
  exit(0);
}
