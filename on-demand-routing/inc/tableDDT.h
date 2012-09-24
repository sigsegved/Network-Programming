/*
 * tableDDT.h
 *
 *  Created on: 10-Nov-2011
 *      Author: dell
 */

#ifndef TABLEDDT_H_
#define TABLEDDT_H_

#include "hw_addrs.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>

#define MAX_IP_LEN 32
#define MAX_MAC_LEN 32
#define MAX_TIME_LEN 100
#define STALENESS_THRSHLD 60 		//Has to be recvd as cmd line arguement....

typedef struct _routeInfo
{
	char dest_ipaddr[MAX_IP_LEN];
	unsigned char  nexthop_addr[MAX_MAC_LEN];
	int hopCount;
	int broadcastID;
	time_t timestamp;
	int ifid;
	int isSelf;
}routeInfo;

typedef struct _routeNode
{
	routeInfo rtinfo;
	struct _routeNode *next;
}routeNode;

typedef struct _odrHeader
{
  unsigned long dest_addr; //changed paddr to unsigned long as that is how the address is stored
  unsigned long src_addr;
  int hopcount;
  time_t timeStamp;
  int broadcastID;
  int reqType;
  int msglen;
}odrHeader;


int addrouteInfo(routeInfo *rtinfo);
routeInfo * getrouteInfo(unsigned int destIP);
void destroyRouteInformation(unsigned int destIP);
void destroyList();
int isStale(unsigned int);


#endif /* TABLEDDT_H_ */
