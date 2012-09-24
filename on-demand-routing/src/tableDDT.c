/*
 * tableDDT.c
 *
 *  Created on: 10-Nov-2011
 *      Author: dell
 */

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
			if((node_found) && (tmp->rtinfo.hopCount > rtinfo->hopCount))
				memcpy(&tmp->rtinfo,rtinfo,sizeof(routeInfo));
			else if(!tmp)
			{
				tmp=routeList;
				newnode->next = tmp;
				routeList = newnode;
			}
			else
			{
				printf("An efficient route already exist for destination : %s\n",rtinfo->dest_ipaddr);
				return 0;
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

int isStale(unsigned int dest_ip)
{

	routeInfo *rtinfo;
	int staleness;

	rtinfo = getrouteInfo(dest_ip);

	time_t curr_time;
	time(&curr_time);

	int diff_secs = difftime(rtinfo->timestamp,curr_time);

	if(diff_secs >= STALENESS_THRSHLD)
		staleness = 1;
	else
		staleness = 0;

	free(rtinfo);
	rtinfo = NULL;

	return staleness;
}
