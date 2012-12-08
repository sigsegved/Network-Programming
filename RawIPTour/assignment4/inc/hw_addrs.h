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

#include <sys/un.h>

#define	IF_NAME		16	/* same as IFNAMSIZ    in <net/if.h> */
#define	IF_HADDR	 6	/* same as IFHWADDRLEN in <net/if.h> */

#define	IP_ALIAS  	 1	/* hwa_addr is an alias */

#define MAX_IP_LEN 32
#define MAX_MAC_LEN 32
#define MAX_TIME_LEN 100
#define STALENESS_THRSHLD 60            //Has to be recvd as cmd line arguement....
#define GRP8_P_ARP 0xAA61
#define HW_HARD_TYPE 1
#define IP_PROT_TYPE 0x0800
#define OP_ARP_REQ 1
#define OP_ARP_REP 2

#define SOCK_COM_FILE "server.sock.uk"

struct hwa_info {
  char    if_name[IF_NAME];	/* interface name, null terminated */
  char    if_haddr[IF_HADDR];	/* hardware address */
  int     if_index;		/* interface index */
  short   ip_alias;		/* 1 if hwa_addr is an alias IP address */
  short   ip_loop;              /* RM - ADDED: 1 if its the loopback address */
  struct  sockaddr  *ip_addr;	/* IP address */
  struct  hwa_info  *hwa_next;	/* next of these structures */
};


typedef struct _hwaddr{
     int             sll_ifindex;        /* Interface number */
     unsigned short  sll_hatype;         /* Hardware type */
     unsigned char   sll_halen;          /* Length of address */
     unsigned char   sll_addr[6];        /* Physical layer address */
}hwaddr;


typedef struct _routeInfo
{
        char dest_ipaddr[MAX_IP_LEN];
        hwaddr dest_mac;
        int isSelf;
        int tour_sockfd;
        struct sockaddr_un cli_addr;
}routeInfo;

typedef struct _routeNode
{
        routeInfo rtinfo;
        struct _routeNode *next;
}routeNode;


typedef struct _arprr
{
        int hardtype;
        int prottype;
        int hardsize;
        int protsize;
        int op;
        unsigned char src_mac[6];
        unsigned long src_ip;
        unsigned char dest_mac[6];
        unsigned long dest_ip;
}arprr;




/* function prototypes */
#endif
