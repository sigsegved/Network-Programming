/*
 * handlers.h
 *
 *  Created on: 20-Nov-2011
 *      Author: dell
 */

#ifndef HANDLERS_H_
#define HANDLERS_H_

#include "hw_addrs.h"
#include "tableDDT.h"
#include "packetQ.h"
#include "odr_client.h"

#define true 1
#define false 0

#define RREQ 0
#define RREP 1
#define PLP 2

void odrMsgHandler(pkt *p,struct sockaddr_ll *rcv_sa,int sockfd, int ipcfd);
void handlePLP(pkt *p,struct sockaddr_ll *rcv_sa,int sockfd, int ipcfd);
void handleRREP(pkt *p,struct sockaddr_ll *rcv_sa,int sockfd);
void handleRREQ(pkt *p,struct sockaddr_ll *rcv_sa,int sockfd);
void checkRoute(odrHeader *ohdr,struct ethhdr *,routeInfo **rinfo_d, routeInfo **rinfo_s,struct sockaddr_ll *rcv_sa, int *route_exist,int *);
void * framepacket(routeInfo *rinfo,odrHeader *ohdr,odr_client_com pldata,int *plen,struct sockaddr_ll *,int,struct ether_addr *,int);
void sendRREP(routeInfo *rinfo,pkt *p,struct sockaddr_ll *rcv_sa,int sockfd,int);
void forwardPLP(routeInfo *rinfo, pkt *p, int sockfd);
void broadcastRREQ(pkt *p,struct sockaddr_ll *rcv_sa,int sockfd);
char *getIP(unsigned int);
#endif /* HANDLERS_H_ */
