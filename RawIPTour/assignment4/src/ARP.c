#include "arp.h"
#include "tableDDT.h"

#define MAX_PACKET_SIZE 1024
struct hwa_info *global_hw_head=NULL;
struct in_addr cannon_ip; //cannonical IP address for me

static void SIGCHLD_HANDLER(int signo);

int main(int argc, char *argv[])
{
  //char buff[MAX_PACKET_SIZE], *payload;
  char buff[4096], *src, *dst;
  //Hardware addressing structure
  struct hwa_info *hwahead = NULL, *interface, *hwa;
  struct sockaddr_ll sa, rcv_sa;
  struct ethhdr rcv_eth, dst_eth;
  arprr rcv_arp;

  socklen_t len = sizeof(struct sockaddr_ll);
  int sockfd = 0, maxfd = 0;
  int nbytes;
  fd_set rset;

  struct sigaction act;

  memset(buff, 0, 4096);
  memset(&sa, 0, sizeof(struct sockaddr_ll));
  memset(&rcv_sa, 0, sizeof(struct sockaddr_ll));
  //memset(&odrhead, 0, sizeof(struct _odrHeader));
  //memset(&rcv_odr, 0, sizeof(struct _odrHeader));
  memset(&rcv_eth, 0, sizeof(struct ethhdr));
  memset(&dst_eth, 0, sizeof(struct ethhdr));
  memset(&cannon_ip, 0, sizeof(struct in_addr));
  

  //ipcfd = server_bind();
  /*********************** START ARGUMENT HANDLING ***************/
  /*********************** END ARGUMENT HANDLING ***************/

  /************ START ZOMBIE KILLING  ***************/
  memset(&act, 0, sizeof(act));

  act.sa_handler = SIGCHLD_HANDLER; //initializes the signal handler struct
  
  sigemptyset(&act.sa_mask);
  
  if (sigaction(SIGCHLD, &act, 0)) //sets signal action
    {
      perror ("ARP: sigaction");
      exit(1);
    }
  /************ END ZOMBIE KILLING  ***************/
  printf("ARP: Getting Hardware Information\n");
  /************ START HARDWARE INFORMATION QUERY ***************/
  hwahead = get_hw_addrs();

  global_hw_head = hwahead;
  
  if(hwahead != NULL)
    PrintAddrs(hwahead);
  else
    {
      printf("ARP: hwahead = NULL\n");
      exit(1);
    }
  
   for (hwa = hwahead; hwa != NULL; hwa = hwa->hwa_next) 
     {
       if(hwa->ip_alias == 0 && hwa->ip_loop == 0 && hwa->if_name[3] == '0')
	 {
	   memcpy(&cannon_ip, &((struct sockaddr_in *)hwa->ip_addr)->sin_addr, sizeof(struct in_addr));
	   interface = hwa;
	   break;
	 }
     }

	sockfd = socket(PF_PACKET, SOCK_RAW, htons(GRP8_P_ARP));
	printf("\n\n SOCKFD : %d \n",sockfd);

	if(sockfd == -1)
	{
		perror("SOCK_ERR ");
		exit(-1);
	}
	printf("ARP: Intializing sockaddr_ll header\n");
	sa.sll_family = AF_PACKET;
	sa.sll_protocol = htons(GRP8_P_ARP);
	sa.sll_ifindex = interface->if_index;
	sa.sll_hatype = 0; //gets set automattically on the receiver side
	sa.sll_pkttype = 0; //gets set automattically on the receiver side
	sa.sll_halen = 6;
	
	
	

	//create a sun path socket
	//bind to a well known sun path file.
	char tbuff[1024];

	int api_sockfd = create_unix_dgram_socket();
	int sd2;
	struct sockaddr_un tour_sa;
	printf("ARP: Entering processing loop\n");

	int tour_sockfd=-1;
	while(1)
	{
		FD_ZERO(&rset);
		FD_SET(sockfd, &rset);
		FD_SET(api_sockfd, &rset);

		if(sockfd > api_sockfd)
			maxfd = sockfd + 1;
		else
			maxfd = api_sockfd + 1;

		if(tour_sockfd>0)
		{
			FD_SET(tour_sockfd,&rset);
			maxfd = tour_sockfd>maxfd?tour_sockfd:maxfd;
		}

		maxfd = maxfd + 1;
				
		printf("ARP: Waiting for data...\n");
		select(maxfd, &rset, NULL, NULL, NULL);


		if(FD_ISSET(api_sockfd,&rset))
		{
			printf("ARP: Connection request from tour module...\n");
			sd2 = accept(api_sockfd, NULL, NULL);
			if (sd2 < 0)
   				perror("accept() failed");
			else
			{
				tour_sockfd = sd2;
				printf("ARP : Now monitoring the TOUR SOCK - %d\n",tour_sockfd);

			}
		}
	if(tour_sockfd>2)
		if(FD_ISSET(tour_sockfd,&rset))
		{

			printf("ARP : RECVD Data from TOUR Module \n");

			memset(tbuff,0,sizeof(tbuff));
			//nbytes = recvfrom(sd2, tbuff, 4096, 0, (struct sockaddr *)&tour_sa, &len);
			nbytes = recv(sd2, tbuff, 4096, 0);
			if(nbytes>0)
			{
				struct sockaddr_in ipaddr;
				memset(&ipaddr,0,sizeof(ipaddr));
				memcpy(&ipaddr,tbuff,sizeof(struct sockaddr_in));
				routeInfo *rinfo;
				rinfo  = getrouteInfo(ipaddr.sin_addr.s_addr);

				if(rinfo)
				{
					printf("MAC ADDRESS FOUND FOR IP \n");
		            //int nbytes = sendto(sd2,&rinfo->dest_mac,sizeof(hwaddr),0,(struct sockaddr *)&tour_sa,sizeof(struct sockaddr_un));
		            int nbytes = send(sd2,&rinfo->dest_mac,sizeof(hwaddr),0);
					if(nbytes<0)
						perror("SENDTO : ");
					else	
   			        	printf("ARP : Sent %d bytes to tour module \n",nbytes);
	
				}
				else
				{
					arprr brd_arp;
					struct ethhdr brd_eth;
		
					addARPCache(sa,interface,ipaddr.sin_addr.s_addr,NULL,&tour_sa,sd2);
					memset(&brd_arp,0,sizeof(brd_arp));
					memset(&brd_eth,0,sizeof(brd_eth));
					memcpy(brd_eth.h_dest,(void *)ether_aton("ff:ff:ff:ff:ff:ff"),6);
  					memcpy(brd_eth.h_source, interface->if_haddr, 6);
					brd_eth.h_proto = htons(GRP8_P_ARP);
					brd_arp.src_ip = ((struct sockaddr_in *)interface->ip_addr)->sin_addr.s_addr;
					brd_arp.dest_ip = ipaddr.sin_addr.s_addr;
					memcpy(brd_arp.src_mac,brd_eth.h_source,6);
					brd_arp.op = OP_ARP_REQ;
   		           	sendARP(brd_arp,brd_eth,sockfd,interface->if_index);
				}
			}
			else
			{
				perror("ARP-ERR : ");
				printf("ARP : TOUR MODULE MUST HAVE TIMED-OUT/CLOSED!! \n");
				printf("ARP : CLOSING SOCKET TO TOUR MODULE!! \n");
				close(tour_sockfd);
				deleteTourInformation(tour_sockfd,1);
				tour_sockfd = -1;
			}
			
		}
		if(FD_ISSET(sockfd,&rset))
		{
			printf("ARP: Data received from vm...\n");
			memset(buff,0,sizeof(buff));
			nbytes = recvfrom(sockfd, buff, 4096, 0, (struct sockaddr *)&rcv_sa, &len);
			memcpy(&rcv_eth, buff, sizeof(struct ethhdr));
			memcpy(&rcv_arp, buff + sizeof(struct ethhdr), sizeof(arprr));
			src = ether_ntoa((struct ether_addr *)&(rcv_eth.h_source));
			printf("ODR: src_mac: %s\t", src);
			dst = ether_ntoa((struct ether_addr *)&(rcv_eth.h_dest));
			printf("dst_mac: %s\tproto:%x\n", dst, ntohs(rcv_eth.h_proto));
			printf("ODR: Header:\n");
			PrintHeader(rcv_arp);
			handleARPmsg(rcv_arp,rcv_eth,sockfd,interface,rcv_sa);
		}
	}
  free_hwa_info(hwahead);
  exit(0);
}

void PrintHeader(arprr head)
{
  char str[512];
  printf("\tsrc ip= %s", inet_ntop(AF_INET, &head.src_ip, str, INET_ADDRSTRLEN));
  memset(str,0,sizeof(str));
  printf("\tdst ip= %s\n",inet_ntop(AF_INET, &head.dest_ip, str, INET_ADDRSTRLEN));
  printf("\tsrc_mac = %s",ether_ntoa((struct ether_addr *)&head.src_mac));
  printf("\tdst_mac = %s\n",ether_ntoa((struct ether_addr *)&head.dest_mac));
  if(head.op == OP_ARP_REQ)
	printf("\ttype = ARP_REQ\n");
  else if(head.op == OP_ARP_REP)
	printf("\ttype = ARP_REP\n");
}

static void SIGCHLD_HANDLER(int signo) //handler for when child processes terminate
{
  pid_t pid;
  int stat;
  
  while((pid = waitpid(-1, &stat, WNOHANG)) > 0) //reap exit status
    write(1, "ARP: Child terminated\n", 17);
  return;
}

void handleARPmsg(arprr rcv_arp, struct ethhdr rcv_eth,int sockfd,struct hwa_info *interface,struct sockaddr_ll rcv_sa)
{

	switch(rcv_arp.op)
	{
		case OP_ARP_REQ:
			arpReqHandler(rcv_arp,rcv_eth,sockfd,interface,rcv_sa);
			break;
		case OP_ARP_REP:
			arpRepHandler(rcv_arp,rcv_eth,sockfd,interface,rcv_sa);
			break;
	}
}

void arpReqHandler(arprr rcv_arp, struct ethhdr rcv_eth,int sockfd,struct hwa_info *interface,struct sockaddr_ll rcv_sa)
{

	arprr tmp_arp;
	char ipaddr[20];
	routeInfo *rinfo;
	//if((strcmp(getIP(rcv_arp.dest_ip),getIP(cannon_ip.s_addr)))==0)
	if(rcv_arp.dest_ip == cannon_ip.s_addr)
	{
		printf("\nARP : ARP REQ Destined to this vm");
		printf("\nARP : Adding Hardware info into the ARP-CACHE");
		addARPCache(rcv_sa,interface,rcv_arp.src_ip,rcv_arp.src_mac,NULL,-1);
		memset(ipaddr,0,sizeof(ipaddr));
		strcpy(ipaddr,getIP(rcv_arp.dest_ip));
		rinfo = getrouteInfo(rcv_arp.dest_ip);
		//add arp info into cache for source ip and source mac
		//send arp rep to src ip with dest mac as etho0 mac
		struct ethhdr tmp;
		memcpy(&tmp,&rcv_eth,sizeof(struct ethhdr));	
		//memcpy(&rcv_eth.h_source,interface->sa_addr,sizeof(struct ether_addr));
		//memcpy(rcv_eth.h_source, (void *)ether_aton(rinfo->nexthop_addr),6);
		memcpy(rcv_eth.h_source,rinfo->dest_mac.sll_addr,6);
		memcpy(rcv_eth.h_dest,tmp.h_source,sizeof(struct ether_addr));
		memcpy(&tmp_arp,&rcv_arp,sizeof(arprr));
		rcv_arp.src_ip=rcv_arp.dest_ip;
		rcv_arp.dest_ip=tmp_arp.src_ip;
		memcpy(rcv_arp.src_mac,rinfo->dest_mac.sll_addr,6);
		memcpy(rcv_arp.dest_mac,rcv_eth.h_dest,sizeof(rcv_arp.src_mac));
		rcv_arp.op =  OP_ARP_REP;

		sendARP(rcv_arp,rcv_eth,sockfd,rinfo->dest_mac.sll_ifindex);
		//add the info into ARP CACHE
		//send ARP REP
		
	}
	else
		printf("\nARP : ARP REQ not destined to this vm");
}

int addARPCache(struct sockaddr_ll rcv_sa,struct hwa_info *interface,unsigned int ip_addr,unsigned char *mac_addr,struct sockaddr_un *cli_addr,int sockfd)
{
	routeInfo rinfo;
	int rc=-1;
	memset(&rinfo,0,sizeof(rinfo));
	strcpy(rinfo.dest_ipaddr,getIP(ip_addr));
	rinfo.dest_mac.sll_ifindex=rcv_sa.sll_ifindex;
	rinfo.dest_mac.sll_ifindex=interface->if_index;
	rinfo.dest_mac.sll_hatype=rcv_sa.sll_hatype;
	rinfo.dest_mac.sll_halen=rcv_sa.sll_halen;
	rinfo.dest_mac.sll_ifindex = interface->if_index;
	rinfo.isSelf = 0;

	if(mac_addr)
		memcpy(rinfo.dest_mac.sll_addr,mac_addr,6);
	else
		printf("ARP : Creating dummy entry for IP : %s ",getIP(ip_addr));

	if(cli_addr)
		memcpy(&rinfo.cli_addr,cli_addr,sizeof(struct sockaddr_un));
	
	if(sockfd!=-1)
		rinfo.tour_sockfd = sockfd;

	rc=addrouteInfo(&rinfo);
	return rc;
}

void arpRepHandler(arprr rcv_arp, struct ethhdr rcv_eth,int sockfd,struct hwa_info *interface,struct sockaddr_ll rcv_sa)
{
    if(rcv_arp.dest_ip == cannon_ip.s_addr)
    {
        printf("\nARP : ARP REP Destined to this vm\n");
        printf("\nARP : Adding Hardware info into the ARP-CACHE\n");
	char *dst_mac;
	dst_mac = ether_ntoa((struct ether_addr *)&(rcv_eth.h_source));
	addARPCache(rcv_sa,interface,rcv_arp.src_ip,rcv_arp.src_mac,NULL,-1);

	routeInfo *rinfo;
	rinfo = getrouteInfo(rcv_arp.src_ip);

	if(rinfo->tour_sockfd > 2 )
	{
	    struct sockaddr_un serv_addr;
	    memset(&serv_addr,0,sizeof(serv_addr));
		int nbytes = send(rinfo->tour_sockfd,&rinfo->dest_mac,sizeof(hwaddr),0);
		printf("ARP : Sent %d bytes to tour module \n",nbytes);
	}
	else
	{
		printf("ARP : ARP-REP recvd for a timed out tour module. \n");
		printf("ARP : Deleting the entry created before for ip %s\n",getIP(rcv_arp.src_ip));
		destroyRouteInformation(rcv_arp.src_ip);
	}

    }
    else
        printf("\nARP : ARP REP not destined to this vm");
}

int sendARP(arprr rcv_arp, struct ethhdr rcv_eth,int sockfd,int ifid)
{
  char buff[1024];
  char *ptr;
  int nbytes;
  struct sockaddr_ll sa;
  socklen_t len = sizeof(struct sockaddr_ll);
  memset(buff,0,sizeof(buff));


  sa.sll_family = AF_PACKET;
  sa.sll_protocol = htons(GRP8_P_ARP);
  sa.sll_ifindex = ifid;
  sa.sll_hatype = 0;
  sa.sll_pkttype = 0;
  sa.sll_halen = 6;
  memcpy(sa.sll_addr, rcv_eth.h_dest, 6);

  char *src,*dst;
  src = ether_ntoa((struct ether_addr *)&(rcv_eth.h_source));
  printf("\nARP : src_mac: %s\t", src);
  dst = ether_ntoa((struct ether_addr *)&(rcv_eth.h_dest));
  printf("dst_mac: %s\tproto:%x\n", dst, ntohs(rcv_eth.h_proto));
  printf("ifid : %d \n",ifid);
  printf("SOCKFD : %d \n",sockfd);
  printf("ARP: Header:\n");
  PrintHeader(rcv_arp);

  ptr = buff;
  memcpy(ptr, &rcv_eth, sizeof(struct ethhdr));
  ptr = ptr + sizeof(struct ethhdr);
  memcpy(ptr, &rcv_arp, sizeof(arprr));
  ptr = ptr + sizeof(arprr);
  nbytes = sendto(sockfd, buff, 1023, 0, (struct sockaddr *)&sa, len);
  if(nbytes < 0)
    {
      perror("sendto");
    }
  printf("%i bytes sent\n", nbytes);
  return nbytes;

	
}


char * getIP(unsigned int ip)
{
        struct in_addr ipaddr;
        ipaddr.s_addr = ip;
        return (inet_ntoa(ipaddr));
}

int create_unix_dgram_socket()
{
    int sockfd = socket(AF_UNIX,SOCK_STREAM,0);

    if(sockfd == -1)
    {
        perror("ARP-API  : ");
        return -1;
    }

    struct sockaddr_un serv_addr;

    memset(&serv_addr,0,sizeof(serv_addr));
	unlink(SOCK_COM_FILE);
    serv_addr.sun_family = AF_UNIX;
    strcpy(serv_addr.sun_path,SOCK_COM_FILE);


    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(struct sockaddr_un)))
    {
        printf("Error binding a datagram socket");
        return -1;
    }

    int rc = listen(sockfd, 10);
    if (rc< 0)
    {
        perror("listen() failed");
	return -1;
    }
return sockfd;

}

