#ifndef __HW_ADDRS_H
#define __HW_ADDRS_H

/* Our own header for the programs that need hardware address info. */

#include <sys/fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/ioctl.h>

#include <arpa/inet.h>

#include <netinet/in.h>
#include <netinet/ether.h>

#include <net/if.h>

#include <asm/types.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h> 

#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <ifaddrs.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#define	IF_NAME		16	/* same as IFNAMSIZ    in <net/if.h> */
#define	IF_HADDR	 6	/* same as IFHWADDRLEN in <net/if.h> */

#define	IP_ALIAS  	 1	/* hwa_addr is an alias */

struct hwa_info {
  char    if_name[IF_NAME];	/* interface name, null terminated */
  char    if_haddr[IF_HADDR];	/* hardware address */
  int     if_index;		/* interface index */
  short   ip_alias;		/* 1 if hwa_addr is an alias IP address */
  short   ip_loop;              /* RM - ADDED: 1 if its the loopback address */
  struct  sockaddr  *ip_addr;	/* IP address */
  struct  hwa_info  *hwa_next;	/* next of these structures */
};

#define GRP8_P_ODR 0xAA59

/* function prototypes */
struct hwa_info	*get_hw_addrs();
struct hwa_info	*Get_hw_addrs();
void	free_hwa_info(struct hwa_info *);
void PrintAddrs(struct hwa_info *head);
int getInterfaceInfo(int,unsigned char *,char *,struct hwa_info *);
#endif
