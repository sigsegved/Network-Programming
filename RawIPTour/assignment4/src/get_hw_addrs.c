#include <errno.h>		/* error numbers */
#include <sys/ioctl.h>          /* ioctls */
#include <net/if.h>             /* generic interface structures */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hw_addrs.h"
#include "hw_func.h"
#include "tableDDT.h"

int max_interface;


struct hwa_info *
get_hw_addrs()
{
  struct ifaddrs *ifaddr, *ifa;
  struct ifreq ifrcopy;
  struct hwa_info *hwa, *head, *hprev;
  int family, sockfd;
  char host[NI_MAXHOST], *cptr, lastname[IF_NAME];
  
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  
  if(getifaddrs(&ifaddr) == -1)
    {
      perror("GetAddrs - getifaddr");
      exit(1);
    }
  
  head = (struct hwa_info *) calloc(1, sizeof(struct hwa_info));
  head->hwa_next = NULL;
  hwa = head;
  hprev = head;
  
  lastname[0] = 0;
  for(ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
    {
      if(ifa->ifa_addr == NULL)
	continue;
      family = ifa->ifa_addr->sa_family;
      
      if(family == AF_INET)
	{
	  if(hwa == NULL)
	    {
	      hwa = (struct hwa_info *) calloc(1, sizeof(struct hwa_info));
	      hprev->hwa_next = hwa;
	      hwa->hwa_next = NULL;
	    }
	  
	  if(getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST) != 0)
	    {
	      perror("GetAddrs - getnameinfo");
	      exit(1);
	    }
	  memcpy(hwa->if_name, ifa->ifa_name, IF_NAME);
	  hwa->if_name[IF_NAME -1] = '\0';
	  
	  if ( (cptr = (char *) strchr(ifa->ifa_name, ':')) != NULL)
	    *cptr = 0;		/* replace colon will null */
	  if (strncmp(lastname, ifa->ifa_name, IF_NAME) == 0) {
	    hwa->ip_alias = IP_ALIAS;
	  }
	  memcpy(lastname, ifa->ifa_name, IF_NAME);
	  
	  hwa->ip_addr = (struct sockaddr *) calloc(1, sizeof(struct sockaddr_in));
	  memcpy(hwa->ip_addr, ifa->ifa_addr, sizeof(struct sockaddr_in));
	  
	  memset(&ifrcopy, 0x00, sizeof(ifrcopy));
	  strcpy(ifrcopy.ifr_name, hwa->if_name);
	  ioctl(sockfd, SIOCGIFHWADDR, &ifrcopy);
	  memcpy(&hwa->if_haddr, &ifrcopy.ifr_hwaddr.sa_data, IF_HADDR);
	  
	  ioctl(sockfd, SIOGIFINDEX, &ifrcopy);
	  memcpy(&hwa->if_index, &ifrcopy.ifr_ifindex, sizeof(int)); 
	  
	  if((ifa->ifa_flags & IFF_LOOPBACK) != 0)
	    hwa->ip_loop = 1;
	  
	  hprev = hwa;
	  hwa = hwa->hwa_next;
	}
    }
  close(sockfd);
  freeifaddrs(ifaddr);
  return head;
}

void
free_hwa_info(struct hwa_info *hwahead)
{
  struct hwa_info	*hwa, *hwanext;
  
  for (hwa = hwahead; hwa != NULL; hwa = hwanext) {
    free(hwa->ip_addr);
    hwanext = hwa->hwa_next;	/* can't fetch hwa_next after free() */
    free(hwa);			/* the hwa_info{} itself */
  }
}
/* end free_hwa_info */

struct hwa_info *
Get_hw_addrs()
{
  struct hwa_info	*hwa;
  
  if ( (hwa = get_hw_addrs()) == NULL)
    {
      perror("get_hw_addrs error");
      exit(1);
    }
  return(hwa);
}

void PrintAddrs(struct hwa_info *head)
{
  struct hwa_info *hwa;
  struct sockaddr *sa;
  struct ether_addr addr;
  char str[INET_ADDRSTRLEN];
  char *hw;

  printf("\n");  

  routeInfo rinfo;

  int interface_count = 0;	

  for (hwa = head; hwa != NULL; hwa = hwa->hwa_next) 
  {
      
    printf("%s :%s", hwa->if_name, ((hwa->ip_alias) == IP_ALIAS) ? " (alias)\n" : "\n");
    
    if ( (sa = hwa->ip_addr) != NULL)
      printf("\tIP addr = %s\n", inet_ntop(AF_INET, &((struct sockaddr_in *)sa)->sin_addr, str, INET_ADDRSTRLEN));
    
    memcpy(addr.ether_addr_octet, hwa->if_haddr, IF_HADDR);
    hw = ether_ntoa(&addr);
    printf("\tHw addr = %s", hw);
    
    printf("\n\tinterface index = %d\n\n", hwa->if_index);
    memset(&rinfo,0,sizeof(routeInfo));
    strcpy(rinfo.dest_ipaddr,inet_ntop(AF_INET, &((struct sockaddr_in *)sa)->sin_addr, str, INET_ADDRSTRLEN));
    memcpy(&rinfo.dest_mac.sll_addr,&hwa->if_haddr,6);
    //rinfo.hopCount=0;
    //rinfo.timestamp=0;
    rinfo.dest_mac.sll_ifindex = hwa->if_index;
    rinfo.isSelf = 1;

    if(addrouteInfo(&rinfo))
      printf("ODR : ROUTE INFO ADDED TO ROUTING TABLE...\n");
    else
      printf("ODR : FAILED TO ADD ROUTE INFO TO ROUTING TABLE... \n");

    interface_count++;
    
  }
  
  printf("ODR : INTERFACE COUNT  : %d \n",interface_count);
  max_interface = interface_count;
}

int getInterfaceInfo(int ifid,unsigned char *mac_addr,char  *ip_addr,struct hwa_info *head)
{
 struct hwa_info *hwa;
 struct sockaddr *sa;
 struct ether_addr addr;
 char str[INET_ADDRSTRLEN];
 char *hw;

 int rc = 0;
 for (hwa = head; hwa != NULL; hwa = hwa->hwa_next)
 {
   if((hwa->if_index == ifid) && (hwa->ip_alias != IP_ALIAS) &&  hwa->ip_loop == 0 && hwa->if_name[3] != '0')
     {
       printf("HDLR : Interface Info \n");
       memcpy(addr.ether_addr_octet, hwa->if_haddr, IF_HADDR);
       hw = ether_ntoa(&addr);
       strcpy(mac_addr,hw);
       if ( (sa = hwa->ip_addr) != NULL)
	 strcpy(ip_addr ,inet_ntop(AF_INET, &((struct sockaddr_in *)sa)->sin_addr, str, INET_ADDRSTRLEN));
       printf("\tMAC ADDR = %s", hw);
       printf("\tIP ADDR = %s",ip_addr);
       printf("\tIFID = %d\n", hwa->if_index);
       rc = 1;
       break;
     }
 }
 
 return rc;
}
