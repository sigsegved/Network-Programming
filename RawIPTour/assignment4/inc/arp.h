#ifndef _ARP_H
#define _ARP_H

#include "hw_addrs.h"
#include "tableDDT.h"
#include "hw_func.h"


void handleARPmsg(arprr rcv_arp, struct ethhdr rcv_eth,int sockfd,struct hwa_info *intface,struct sockaddr_ll rcv_sa);
void arpReqHandler(arprr rcv_arp, struct ethhdr rcv_eth,int sockfd,struct hwa_info *,struct sockaddr_ll);
int sendARP(arprr rcv_arp, struct ethhdr rcv_eth,int sockfd,int ifid);
void arpRepHandler(arprr rcv_arp, struct ethhdr rcv_eth,int sockfd,struct hwa_info *,struct sockaddr_ll);

char * getIP(unsigned int ip);
int create_unix_dgram_socket();
void PrintHeader(arprr head);
int addARPCache(struct sockaddr_ll rcv_sa,struct hwa_info *interface,unsigned int ip_addr,unsigned char *mac_addr,struct sockaddr_un *,int );

#endif
