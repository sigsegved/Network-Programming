#include "../inc/hw_addrs.h"
#include "../inc/tableDDT.h"
#include "../inc/odr_client.h"
#include "handlers.h"
#include "packetQ.h"

#define MAX_PACKET_SIZE 1024

struct hwa_info *global_hw_head=NULL;
struct in_addr cannon_ip; //cannonical IP address for me

static void SIGCHLD_HANDLER(int signo);
void PrintHeader(struct _odrHeader head);

int main(int argc, char *argv[])
{
  //char buff[MAX_PACKET_SIZE], *payload;
  char buff[4096], *msg, *src, *dst;
  //Hardware addressing structure
  struct hwa_info *hwahead = NULL, *interface, *hwa;
  struct sockaddr_ll sa, rcv_sa;
  struct ethhdr rcv_eth, dst_eth;
  struct _odrHeader odrhead, rcv_odr;

  odr_client_com data;

  socklen_t len = sizeof(struct sockaddr_ll);
  int sockfd = 0, ipcfd = 0, maxfd = 0, staleness = 0;
  int nbytes;
  fd_set rset;

  struct sigaction act;

  memset(buff, 0, 4096);
  memset(&sa, 0, sizeof(struct sockaddr_ll));
  memset(&rcv_sa, 0, sizeof(struct sockaddr_ll));
  memset(&odrhead, 0, sizeof(struct _odrHeader));
  memset(&rcv_odr, 0, sizeof(struct _odrHeader));
  memset(&rcv_eth, 0, sizeof(struct ethhdr));
  memset(&dst_eth, 0, sizeof(struct ethhdr));
  memset(&cannon_ip, 0, sizeof(struct in_addr));
  
  srand(time(NULL));

  ipcfd = server_bind();
  /*********************** START ARGUMENT HANDLING ***************/
  if( argc != 2 )
    {
      printf("Usage: ODR <staleness>\n");
      exit(1);
    }

  staleness = atoi(argv[1]);
  if(staleness == 0)
    {
      printf("ODR: Invalid argument\n");
      exit(1);
    }
  /*********************** END ARGUMENT HANDLING ***************/

  /************ START ZOMBIE KILLING  ***************/
  memset(&act, 0, sizeof(act));

  act.sa_handler = SIGCHLD_HANDLER; //initializes the signal handler struct
  
  sigemptyset(&act.sa_mask);
  
  if (sigaction(SIGCHLD, &act, 0)) //sets signal action
    {
      perror ("ODR: sigaction");
      exit(1);
    }
  /************ END ZOMBIE KILLING  ***************/
  printf("ODR: Getting Hardware Information\n");
  /************ START HARDWARE INFORMATION QUERY ***************/
  hwahead = get_hw_addrs();

  global_hw_head = hwahead;
  
  if(hwahead != NULL)
    PrintAddrs(hwahead);
  else
    {
      printf("ODR: hwahead = NULL\n");
      exit(1);
    }
  
   for (hwa = hwahead; hwa != NULL; hwa = hwa->hwa_next) 
     {
       if(hwa->ip_alias == 0 && hwa->ip_loop == 0 && hwa->if_name[3] == '0')
	 {
	   memcpy(&cannon_ip, &((struct sockaddr_in *)hwa->ip_addr)->sin_addr, sizeof(struct in_addr));
	 }
       if(hwa->ip_alias == 0 && hwa->ip_loop == 0 && hwa->if_name[3] != '0')
	 {
	   interface = hwa;
	   break;
	 }
     }
  /*********************** END HARDWARE QUERY ***************/
   printf("ODR: Creating socket\n");
  /*********************** CREATE SOCKET ***************/
  //Uncomment the following line when we are able to run this as root.
  //Wont work if you aren't root!
  sockfd = socket(PF_PACKET, SOCK_RAW, htons(GRP8_P_ODR));

  //memcpy(eth_header.h_dest, (void *)ether_aton("ff:ff:ff:ff:ff:ff"),6);
  //memcpy(eth_header.h_source, interface->if_haddr, 6);
  //eth_header.h_proto = htons(GRP8_P_ODR);

  printf("ODR: Intializing sockaddr_ll header\n");
  sa.sll_family = AF_PACKET;
  sa.sll_protocol = htons(GRP8_P_ODR);
  sa.sll_ifindex = interface->if_index;
  sa.sll_hatype = 0; //gets set automattically on the receiver side
  sa.sll_pkttype = 0; //gets set automattically on the receiver side
  sa.sll_halen = 6;

  printf("ODR: Intializing odr header\n");
  //sa.sll_addr = interface->if_haddr; !DESTINATION ADDRESS
  odrhead.src_addr = ((struct sockaddr_in *)interface->ip_addr)->sin_addr.s_addr;
  odrhead.hopcount = 0;
   
  if(sockfd > ipcfd)
    maxfd = sockfd + 1;
  else
    maxfd = ipcfd + 1;

  FD_ZERO(&rset);
  
  printf("ODR: Entering processing loop\n");
  while(1)
    {
      FD_SET(sockfd, &rset);
      FD_SET(ipcfd, &rset);  //reenable for unix domain sockets
      printf("ODR: Waiting for data...\n");
      select(maxfd, &rset, NULL, NULL, NULL);
      if(FD_ISSET(sockfd, &rset)) //Network request
	{
	  printf("ODR: Data received...\n");
	  memset(buff,0,sizeof(buff));
	  nbytes = recvfrom(sockfd, buff, 4096, 0, (struct sockaddr *)&rcv_sa, &len);

	  printf("ODR: %i bytes\tprotocol: %X\n", nbytes, ntohs(rcv_sa.sll_protocol));
	  memcpy(&rcv_odr, buff + sizeof(struct ethhdr), sizeof(struct _odrHeader));
	  memcpy(&rcv_eth, buff, sizeof(struct ethhdr));
	  
	  //INTESTING FACT REGARDING THE FOLLOWING CODE
	  //If youwere to execute the following code:
	  //
	  //src = ether_ntoa((struct ether_addr *)&(rcv_eth.h_source));
	  //dst = ether_ntoa((struct ether_addr *)&(rcv_eth.h_dest));
	  //
	  //src AND dst will be equal to whatever dst points to. It appears
	  //ether_ntoa always returns a pointer to the same address buffer.
	  //so in order to properly use it, you must call it, copy out the string elsewhere
	  //and then call it again
	  src = ether_ntoa((struct ether_addr *)&(rcv_eth.h_source));
	  printf("ODR: src_mac: %s\t", src);
	  dst = ether_ntoa((struct ether_addr *)&(rcv_eth.h_dest));
	  printf("dst_mac: %s\tproto:%x\n", dst, ntohs(rcv_eth.h_proto));
	  printf("ODR: Header:\n");
	  PrintHeader(rcv_odr);
	  
	  msg = buff + sizeof(struct _odrHeader) + sizeof(struct ethhdr);
	  printf("\n");
	  //CALL THE ODR MSG HANDLER HERE....
	  pkt p;
	  memset(&p,0,sizeof(p));
	  memcpy(&(p.ethInfo), &rcv_eth,sizeof(struct ethhdr));
	  memcpy(&(p.odrInfo), &rcv_odr,sizeof(odrHeader));
	
	  if(rcv_odr.msglen > 0)
	    memcpy(&p.data,msg,rcv_odr.msglen);
	  odrMsgHandler(&p,&rcv_sa,sockfd,ipcfd);
	}
       if(FD_ISSET(ipcfd, &rset))  //IPC Request
	 {
	   server_getevent(ipcfd, &data);
	   if (data.event == EVENT_SEND)
	     {
	       printf("Request to send '%s' to '%s:%d'\n", data.msg, data.ip, data.dst_port);
	       pkt *p;
	       //p = calloc(1, sizeof(pkt));
	       p = (pkt *) malloc(sizeof(pkt));
	       routeInfo *r_d = NULL;
	       memset(p, 0, sizeof(pkt));
	       memcpy(&p->data, &data, sizeof(data));
	       inet_pton(AF_INET, data.ip, &p->odrInfo.dest_addr);
	       memcpy(&p->odrInfo.src_addr, &cannon_ip, sizeof(cannon_ip));
	       p->odrInfo.msglen = sizeof(data);
	       p->odrInfo.reqType = PLP;
	       p->odrInfo.timeStamp = time(NULL);
	       p->odrInfo.broadcastID = rand();

	       if(p->odrInfo.src_addr == p->odrInfo.dest_addr) //I'm talking to my self
		 {
			printf("\n ODR : TALKING TO MYSELF BITCH\n");
			printf("\n ODR : src_port = %d \t dst_port = %d \n",data.src_port,data.dst_port);
			printf("\n ODR : data.msg = %s \n",data.msg);
		   server_sendevent(ipcfd, data.ip, data.src_port, data.dst_port, data.msg);
		 }
	       else
		 {
		   r_d = getrouteInfo(p->odrInfo.dest_addr);
		   if(r_d == NULL)
		     {
		       broadcastRREQ(p, NULL, sockfd);
		       insert_pkt(p);
		     }
		   else
		     {
		       memcpy(&p->ethInfo.h_dest, &r_d->nexthop_addr, sizeof(p->ethInfo.h_dest));
		       for (hwa = hwahead; hwa != NULL; hwa = hwa->hwa_next) 
			 {
			   if(hwa->if_index == r_d->ifid)
			     {
			       memcpy(&p->ethInfo.h_source, hwa->if_haddr, 6);
			       break;
			     }
			 }
		       p->ethInfo.h_proto = htons(GRP8_P_ODR);
		       p->odrInfo.hopcount = r_d->hopCount;

		       forwardPLP(r_d, p, sockfd);

		     }
		 }
	     }
	 }
    }
  /*********************** CLEAN UP ***************/
  free_hwa_info(hwahead);
  exit(0);
}

void PrintHeader(struct _odrHeader head)
{
  char str[512];
 // printf("\tsrc = %s\tdst=%s\n", inet_ntop(AF_INET, &head.src_addr, str, INET_ADDRSTRLEN), 
  printf("\tsrc = %s", inet_ntop(AF_INET, &head.src_addr, str, INET_ADDRSTRLEN));
  memset(str,0,sizeof(str));
  printf("\tdst = %s\n",inet_ntop(AF_INET, &head.dest_addr, str, INET_ADDRSTRLEN));

  printf("\thopcount = %i\ttimestamp = %lu\n", head.hopcount, head.timeStamp);
  printf("\treqtype = %i\tmsglen = %i\n", head.reqType, head.msglen);
}

static void SIGCHLD_HANDLER(int signo) //handler for when child processes terminate
{
  pid_t pid;
  int stat;
  
  while((pid = waitpid(-1, &stat, WNOHANG)) > 0) //reap exit status
    write(1, "ODR: Child terminated\n", 17);
  return;
}

