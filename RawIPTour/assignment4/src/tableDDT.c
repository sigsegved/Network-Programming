/*
 * tableDDT.c
 *
 *  Created on: 10-Nov-2011
 *      Author: dell
 */
#include "arp.h"
#include "tableDDT.h"

routeNode *routeList = NULL;
int addrouteInfo(routeInfo *rtinfo)
{
	int node_found=0;
	if(!rtinfo)
	{
		return 0;
	}
	else
	{
		routeNode *newnode = (routeNode *)malloc(sizeof(routeNode));
		memset(newnode,0,sizeof(routeNode));
		memcpy(&newnode->rtinfo,rtinfo,sizeof(routeInfo));
		newnode->next = NULL;
		if(routeList==NULL)
			routeList = newnode;
		else
		{
			routeNode *tmp = routeList; //tmp=1234, routelist=1234
			while(tmp!=NULL)
			{
				if(strcmp(tmp->rtinfo.dest_ipaddr,rtinfo->dest_ipaddr)==0)
				{
					node_found=1;
					break;
				}
				tmp=tmp->next;
			}
			if(node_found)
			{
				struct ether_addr addr;
				memset(&addr,0,sizeof(addr));
				memcpy(&tmp->rtinfo.dest_mac,&rtinfo->dest_mac,sizeof(hwaddr));
	       		memcpy(addr.ether_addr_octet,tmp->rtinfo.dest_mac.sll_addr, IF_HADDR);
				printf("ARP : ARP CACHE UPDATED FOR IP : %s -- %s\n",tmp->rtinfo.dest_ipaddr,ether_ntoa(&addr));
				//printf("ARP : TOUR SOCKFD FOR IP %S : %d\n",tmp->rtinfo.dest_ipaddr,tmp->rtinfo.tour_sockfd);
			}
			else 
			{
				tmp=routeList;
				newnode->next = tmp;
				routeList = newnode;
			}

		}
	}
	return 1;
}

routeInfo * getrouteInfo(unsigned int destIP_addr)
{
	routeNode *tmp;
	tmp = routeList;
	int route_found = 0;
	routeInfo *rtinfo=NULL;

	struct in_addr ip;
	ip.s_addr = destIP_addr;
	char *destIP = inet_ntoa(ip);
	
	while(tmp!=NULL)
	{
		if(!strcmp(tmp->rtinfo.dest_ipaddr,destIP))
		{
			route_found =1;
			break;
		}
		tmp = tmp->next;
	}

	if(route_found && tmp)
	{
		rtinfo = (routeInfo *)malloc(sizeof(routeInfo));
		memcpy(rtinfo,&tmp->rtinfo,sizeof(routeInfo));
	}
	return rtinfo;
}


void deleteTourInformation(int tfd ,int opt)
{
	routeNode *tmp;
	routeNode *prev=NULL;
	tmp = routeList;
	int route_found = 0;
	//struct in_addr ip;
	//char *destIP = inet_ntoa(ip);

	while(tmp!=NULL)
	{
		if(tmp->rtinfo.tour_sockfd == tfd)
		{
			route_found = 1;
			break;
		}
		prev = tmp;
		tmp = tmp->next;
	}
	if(route_found && tmp)
	{

		if(opt==1)
		{
	        if(prev)
   	     	{
          	 	 prev->next=tmp->next;
           		 //printf("REMOVING ROUTE TO IP : %s\n",destIP);
					printf("APR CACHE ENTRY REMOVED\n");
           		 free(tmp);
           		 tmp = NULL;
       		 }
       		 else
       		 {  
       		     free(tmp);
       		     tmp = NULL;
       		     routeList = NULL;
       		 }

		}
		else
		{
			tmp->rtinfo.tour_sockfd = -1;
			memset(&tmp->rtinfo.cli_addr,0,sizeof(struct sockaddr_un));
		}
	}
	
}

void destroyRouteInformation(unsigned int destIP_addr)
{
	routeNode *tmp;
	routeNode *prev=NULL;
	tmp = routeList;
	int route_found = 0;
	struct in_addr ip;
	ip.s_addr = destIP_addr;
	char *destIP = inet_ntoa(ip);

	while(tmp!=NULL)
	{
		if(!strcmp(tmp->rtinfo.dest_ipaddr,destIP))
		{
			route_found = 1;
			break;
		}
		prev = tmp;
		tmp = tmp->next;
	}

	if(route_found && tmp)
	{
		//might be wrong...
		if(prev)
		{
			prev->next=tmp->next;
			printf("REMOVING ROUTE TO IP : %s\n",destIP);
			free(tmp);
			tmp = NULL;
		}
		else
		{	
			free(tmp);
			tmp = NULL;
			routeList = NULL;
		}
	}
	else
		printf("NO ROUTE FOUND FOR IP : %s\n",destIP);
}


void destroyList()
{
	routeNode *tmp;
	routeNode *prev;
	tmp = routeList;

	while(tmp!=NULL)
	{
		prev = tmp;
		tmp = tmp->next;
		printf("REMOVING ROUTE TO IP : %s\n",prev->rtinfo.dest_ipaddr);
		free(prev);
	}
	routeList = NULL;
}

