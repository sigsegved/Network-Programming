#include "../inc/hw_addrs.h"
#include "../inc/arp.h"

#include <sys/fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/ioctl.h>

#include <arpa/inet.h>

#include <netinet/in.h>
#include <netinet/ether.h>

#include <net/if.h>

#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <ifaddrs.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#define MAX_PACKET_SIZE 1024

int main(int argc, char *argv[])
{
  //char buff[MAX_PACKET_SIZE], *payload;
  char buff[4096], *ptr, in_ad[20];
  int int_num = 0;
  //Hardware addressing structure
  struct hwa_info *hwahead = NULL, *interface, *hwa;
  struct sockaddr_ll sa;
  struct ethhdr eth_header;
  arprr arpmsg;
  struct sockaddr_in sd;
  socklen_t len = sizeof(struct sockaddr_ll);
  int sockfd = 0;
  int nbytes;

  if(argc != 4)
    {
      printf("Usage: ARP_TEST <dest_ip> <dest_mac> <interface_id>\n");
      exit(1);
    }
  

  memset(buff, 0, 4096);
  memset(&sa, 0, sizeof(struct sockaddr_ll));
  memset(&arpmsg, 0, sizeof(arprr));
  memset(&eth_header, 0, sizeof(struct ethhdr));
  int_num = atoi(argv[3]);

  printf("ARP: Getting Hardware Information\n");
  /************ START HARDWARE INFORMATION QUERY ***************/
  hwahead = get_hw_addrs();
  
  if(hwahead != NULL)
    PrintAddrs(hwahead);
  else
    {
      printf("ARP: hwahead = NULL\n");
      exit(1);
    }
  
   for (hwa = hwahead; hwa != NULL; hwa = hwa->hwa_next) 
     {
       if(hwa->ip_alias == 0 && hwa->ip_loop == 0 && hwa->if_index == int_num)
	 {
	   interface = hwa;
	   break;
	 }
     }
  /*********************** END HARDWARE QUERY ***************/
   printf("ARP: Creating socket\n");
  /*********************** CREATE SOCKET ***************/
  //Uncomment the following line when we are able to run this as root.
  //Wont work if you aren't root!
  sockfd = socket(PF_PACKET, SOCK_RAW, htons(GRP8_P_ARP));

  //dst_addr = ether_aton("ff:ff:ff:ff:ff:ff"); //48:5b:39:89:a8:8c
	memset(in_ad,0,sizeof(in_ad));
  strcpy(in_ad,argv[2]);
  memcpy(eth_header.h_dest,(void *)ether_aton(in_ad),6);
  memcpy(eth_header.h_source, interface->if_haddr, 6);
  eth_header.h_proto = htons(GRP8_P_ARP);
  
  inet_pton(AF_INET, argv[2], &(sd.sin_addr));

  printf("ARP: Intializing sockaddr_ll header\n");
  sa.sll_family = AF_PACKET;
  sa.sll_protocol = htons(GRP8_P_ARP);
  sa.sll_ifindex = int_num;
  sa.sll_hatype = 0; //gets set automattically on the receiver side
  sa.sll_pkttype = 0; //gets set automattically on the receiver side
  sa.sll_halen = 6;
  memcpy(sa.sll_addr, eth_header.h_dest, 6);


  printf("ARP: Intializing odr header\n");
  //sa.sll_addr = interface->if_haddr; !DESTINATION ADDRESS
  arpmsg.src_ip = ((struct sockaddr_in *)interface->ip_addr)->sin_addr.s_addr;
  arpmsg.dest_ip = sd.sin_addr.s_addr;
  memcpy(arpmsg.src_mac, eth_header.h_dest, 6);
  arpmsg.op = OP_ARP_REQ;

  
  ptr = buff;
  memcpy(ptr, &eth_header, sizeof(struct ethhdr));
  ptr = ptr + sizeof(struct ethhdr);
  memcpy(ptr, &arpmsg, sizeof(arprr));
  ptr = ptr + sizeof(arprr);

  nbytes = sendto(sockfd, buff, 1024, 0, (struct sockaddr *)&sa, len);  
  if(nbytes < 0)
    {
      perror("sendto");
      exit(1);
    }
  printf("%i bytes sent\n", nbytes);
  /*********************** CLEAN UP ***************/
  free_hwa_info(hwahead);
  exit(0);
}



