/*
 * handlers.c
 *
 *  Created on: 18-Nov-2011
 *      Author: Karthik Uthaman
 */

#include "handlers.h"

extern struct hwa_info *global_hw_head;
extern struct in_addr cannon_ip;

//Calls the handler functions based on the message type...

void odrMsgHandler(pkt *p,struct sockaddr_ll *rcv_sa,int sockfd, int ipcfd)
{
	switch(p->odrInfo.reqType)
	{
	case 0:
		printf("HDLR : RREQ PACKET RECVD \n");
		handleRREQ(p,rcv_sa,sockfd);
		break;
	case 1:
		handleRREP(p,rcv_sa,sockfd);
		printf("HDLR : RREP PACKET RECVD \n");
		break;
	case 2:
		printf("HDLR : PAYLOAD PACKET RECVD \n");
		handlePLP(p,rcv_sa,sockfd,ipcfd);
		break;
	default :
		printf("ERROR : UNEXPECTED PACKET\n");
		break;
	};

}

void handlePLP(pkt *p,struct sockaddr_ll *rcv_sa,int sockfd, int ipcfd)
{
  routeInfo *rinfo_d=NULL;
  routeInfo *rinfo_s=NULL;
  
  int route_exist = false;
  int route_exist_src = false;
  checkRoute(&p->odrInfo, &p->ethInfo, &rinfo_d, &rinfo_s, rcv_sa, &route_exist, &route_exist_src);
  
  printf("HDLR : Payload packet recvd...\n");
  if(route_exist)
    {
      //send out packet... to the next hop address...
      //form the packet destined to the nexthop addr...
      //get the ethernet address for the interface number...
      
      if(rinfo_d->isSelf == 1)
	{
	  printf("HDLR : PAYLOAD PACKET DESTINED FOR THIS ODR... \n");
	  printf("HDLR : PACKET SENT TO APPLICATION LAYER...\n");
	  odr_client_com data;
	  memset(&data, 0, sizeof(odr_client_com));
	  memcpy(&data, &p->data, sizeof(odr_client_com));
	  //server_sendevent(ipcfd, data.ip, data.src_port, data.dst_port, data.msg);
                        printf("\n ODR : TALKING TO MYSELF in handlePLP\n");
                        printf("\n ODR : src_port = %d \t dst_port = %d \n",data.src_port,data.dst_port);
                        printf("\n ODR : data.msg = %s \n",data.msg);
	
		
	  server_sendevent(ipcfd, getIP(p->odrInfo.src_addr), data.src_port, data.dst_port, data.msg);
	}
      else
	{
	  printf("HDLR : Forwarding the PL packet to %s\n",rinfo_d->nexthop_addr);
	  forwardPLP(rinfo_d, p, sockfd);
	}
    }
  else
    {
      //broadcast RREQ and queue the packet...
		printf("HDLR : No route exist for the IP : %s\n",getIP(p->odrInfo.dest_addr));
        broadcastRREQ(p, NULL, sockfd);
		printf("HDLR : PL Packet queued for RREP\n");
		insert_pkt(p);
	}
}

void checkRoute(odrHeader *ohdr, struct ethhdr *rcv_eth,routeInfo **rinfo_d, routeInfo **rinfo_s,struct sockaddr_ll *rcv_sa, int *route_exist,int *route_exist_src)
{
  routeInfo *new_route_info;
  
  routeInfo *r_s=NULL;
  routeInfo *r_d=NULL;
  r_s = getrouteInfo(ohdr->src_addr);
  r_d = getrouteInfo(ohdr->dest_addr);
  
  //can we add a route to the node from which we got the packet from???
  if(!r_s)
    {
      *route_exist_src = false;
      new_route_info = (routeInfo *)malloc(sizeof(routeInfo));
      memset(new_route_info,0,sizeof(routeInfo));
      
      new_route_info->broadcastID = ohdr->broadcastID;
      //check data types
      struct in_addr ip;
      ip.s_addr = ohdr->src_addr;
      char *destIP = inet_ntoa(ip);
      strcpy(new_route_info->dest_ipaddr,destIP);
      char *dst_mac = ether_ntoa((struct ether_addr *)&(rcv_eth->h_source));
      strcpy(new_route_info->nexthop_addr,dst_mac);
      new_route_info->hopCount = ohdr->hopcount;
      new_route_info->timestamp= time(NULL);
      new_route_info->ifid = rcv_sa->sll_ifindex;
      
      addrouteInfo(new_route_info);
      printf("ADDING ROUTE DEST : %s --> %s --> %d\n",destIP,dst_mac,new_route_info->ifid);
      r_s = getrouteInfo(ohdr->src_addr);
    }
  else
    *route_exist_src = true;
  
  if(r_d)
    {
      *route_exist=true;
      *rinfo_d = (routeInfo *)malloc(sizeof(routeInfo));
      memset((*rinfo_d),0,sizeof(routeInfo));
      memcpy((*rinfo_d),r_d,sizeof(routeInfo));
      free(r_d);
    }
  else
    {
      *route_exist = false;
      rinfo_d = NULL;
    }
  *rinfo_s = (routeInfo *)malloc(sizeof(routeInfo));
  memset((*rinfo_s),0,sizeof(routeInfo));
  memcpy((*rinfo_s),r_s,sizeof(routeInfo));
  free(r_s);
  
}

void handleRREP(pkt *p,struct sockaddr_ll *rcv_sa,int sockfd)
{
	routeInfo *rinfo_d=NULL;
	routeInfo *rinfo_s=NULL;

	int route_exist = false;
	int route_exist_src = false;
	checkRoute(&p->odrInfo,&p->ethInfo,&rinfo_d,&rinfo_s,rcv_sa,&route_exist,&route_exist_src);
	printf("HDLR : RREP Packet recvd...\n");
	if(route_exist)
	{
		//send RREP iff the route is not stale...
		if(rinfo_d->isSelf)
		{
			printf("HDLR : RREP Packet destined to this ODR\n");
		}
		else if(!isStale(p->odrInfo.dest_addr))
		{
			printf("HDLR : Forwarding RREP to %s\n",rinfo_d->dest_ipaddr);
			sendRREP(rinfo_d,p,rcv_sa,sockfd,0);
		}
		else
		{
			printf("HDLR : Stale Route exist for IP : %s\n",getIP(p->odrInfo.dest_addr));
			printf("HDLR : Broadcasting RREQ \n");
			destroyRouteInformation(p->odrInfo.dest_addr);
			broadcastRREQ(p,rcv_sa,sockfd);
			printf("HDLR : RREP packet queued...\n");
			insert_pkt(p);
		}
	}
	else
	{

		printf("HDLR : No route exist for the IP : %s\n",getIP(p->odrInfo.dest_addr));
		broadcastRREQ(p,rcv_sa,sockfd);
		printf("HDLR : RREP packet queued...\n");
		insert_pkt(p);

	}

	free(rinfo_d);
	rinfo_d = NULL;
	pkt tpkt;
	printf("HDLR : Checking for any queued packets that can be sent out...\n");
	int sent_pkts=0;

	Qpkt *thead = getHead();
	Qpkt *prev = NULL;

	extern Qpkt *Qhead;

	while(thead!=NULL)
	{

		printf("\nPACKET DESTINED TO %s ",getIP(thead->packet.odrInfo.dest_addr));
		rinfo_d=getrouteInfo(thead->packet.odrInfo.dest_addr);
		
		if(rinfo_d)
		{
			printf(" HAS A ROUTE NO\n");
			memset(&tpkt,0,sizeof(pkt));
			memcpy(&tpkt,&thead->packet,sizeof(pkt));
			printf("HDLR : Type %d Packet, sent to %s\n",tpkt.odrInfo.reqType,rinfo_d->dest_ipaddr);
			//frame packet...
			//send to the next hop addr...

			switch(tpkt.odrInfo.reqType)
			{
			case 1: 
				sendRREP(rinfo_d,&tpkt,NULL,sockfd,0);
				break;
			case 2:
				forwardPLP(rinfo_d,&tpkt,sockfd);
				break;
			}
			sent_pkts++;
			free(rinfo_d);
			if(!prev)
			{
				Qhead = thead->nextPkt;
				free(thead);
				thead = Qhead;
			}
			else
			{
				prev->nextPkt = thead->nextPkt;
				free(thead);
				thead = prev;
			}
			
		}
		else
		{
			printf("HAS NO ROUTE\n");
			prev = thead;
			thead = thead->nextPkt;
		}

	
	}

	printf("HDLR : Sent out %d packets which was cached earlier...\n",sent_pkts);
}

void handleRREQ(pkt *p,struct sockaddr_ll *rcv_sa,int sockfd)
{
	routeInfo *rinfo_d=NULL;
	routeInfo *rinfo_s=NULL;

	int duplicate_rreq = false;
	int route_exist = false;
	int route_exist_src = false;
	checkRoute(&p->odrInfo,&p->ethInfo,&rinfo_d,&rinfo_s,rcv_sa,&route_exist,&route_exist_src);

	if(rinfo_s && route_exist_src)
	{
		
		if(rinfo_s->broadcastID == p->odrInfo.broadcastID)
			duplicate_rreq=true;
		if(rinfo_s->isSelf == 1)
			duplicate_rreq=true;
			
	}

	if(duplicate_rreq){
		//ignore it....
		printf("HDLR : DUPLICATE RREQ RECVD...\n");
	}
	else
	{
		if(route_exist)
		{
			//send RREP iff the route is not stale...
			if(!isStale(p->odrInfo.dest_addr))
			{
				printf("HDLR : Sending RREP FOR RREQ...\n");
				//strcpy(rinfo_s->dest_ipaddr,getIP(cannon_ip.s_addr));
				//p->odrInfo.src_addr = cannon_ip.s_addr;
				sendRREP(rinfo_s,p,rcv_sa,sockfd,1);
			}
			else
			{
				printf("HDLR : Stale route exist for dest ip %s\n",getIP(p->odrInfo.dest_addr));
				destroyRouteInformation(p->odrInfo.dest_addr);
				broadcastRREQ(p,rcv_sa,sockfd);
			}
		}
		else
		{
			//broadcast RREQ...
			printf("HDLR : No Route exist for dest IP : %s\n",getIP(p->odrInfo.dest_addr));
			broadcastRREQ(p,rcv_sa,sockfd);
		}
	}
}

void *framepacket(routeInfo *rinfo,odrHeader *ohdr, odr_client_com pldata,int *bufflen,struct sockaddr_ll *saddr,int reqType, struct ether_addr *dmac,int flip)
{
	struct ethhdr eth_header;
	odrHeader *odr_hdr;

	//construct ethernet header...
	memset(&eth_header, 0, sizeof(struct ethhdr));

	unsigned int if_ip;
	unsigned char if_mac[32];
	char ip_addr[32];
	memset(if_mac,0,sizeof(if_mac));
	memset(ip_addr,0,sizeof(if_mac));
	void *ptr=NULL,*tptr=NULL;
	
	int rc = getInterfaceInfo(rinfo->ifid,if_mac,ip_addr,global_hw_head);

	int same = 1;
	char szdmac[32];
	if(dmac)
	{
		memset(szdmac,0,sizeof(szdmac));
		strcpy(szdmac,(char *)ether_ntoa(dmac));
		if(strcmp(if_mac,szdmac) != 0)
			same = 0;

	}
	else
		same = 0;	

	if(rc && (!same))
	{

	//next hop mac address for the destination IP...
	memcpy(eth_header.h_dest, (void *)ether_aton(rinfo->nexthop_addr),6);	

	//ODRs mac address
	memcpy(eth_header.h_source, (void *)ether_aton(if_mac), 6);		

	eth_header.h_proto = htons(GRP8_P_ODR);

	//inet_pton(AF_INET, odr_ip, &(sd.sin_addr));		//use the ODR canonical IP
	//printf("HDLR: Intializing sockaddr_ll header\n");

	struct sockaddr_ll sa;
	sa.sll_family = AF_PACKET;
	sa.sll_protocol = htons(GRP8_P_ODR);
	sa.sll_ifindex = rinfo->ifid;					//why is this hard-coded to 3?
	sa.sll_hatype = 0; 					//gets set automattically on the receiver side
	sa.sll_pkttype = 0; 				//gets set automattically on the receiver side
	sa.sll_halen = 6;					//MAC ADDRESS LEN
	memcpy(sa.sll_addr, eth_header.h_dest, 6);

	memcpy(saddr,&sa,sizeof(sa));

	struct in_addr dest_ip;

	inet_aton(rinfo->dest_ipaddr, &dest_ip);
	inet_pton(AF_INET, ip_addr, &if_ip);

	odr_hdr = (odrHeader *)malloc(sizeof(odrHeader));
	//odr_hdr->src_addr = if_ip;
	if(flip == 1)
	{
		odr_hdr->dest_addr = ohdr->src_addr;
		odr_hdr->src_addr = ohdr->dest_addr;
		
	}
	else
	{
	//odr_hdr->dest_addr = dest_ip.s_addr;
	
	odr_hdr->src_addr = ohdr->src_addr;
	odr_hdr->dest_addr = ohdr->dest_addr;
	}
	
	odr_hdr->hopcount = rinfo->hopCount;
	odr_hdr->msglen = ohdr->msglen;
	odr_hdr->reqType = reqType;
	odr_hdr->timeStamp = time(NULL);
	odr_hdr->broadcastID = ohdr->broadcastID;

	int plen;
	if(odr_hdr->msglen > 0)
		plen = sizeof(struct ethhdr)+sizeof(odrHeader)+odr_hdr->msglen;
	else
		plen = sizeof(struct ethhdr)+sizeof(odrHeader);

	ptr=(void *)malloc(plen);
	tptr = ptr;
	memset(ptr,0,plen);
	memcpy(ptr, &eth_header, sizeof(struct ethhdr));
	ptr = ptr + sizeof(struct ethhdr);
	memcpy(ptr, odr_hdr, sizeof(odrHeader));
	ptr = ptr + sizeof(odrHeader);

	if(odr_hdr->msglen > 0)
	{
		memcpy(ptr,&pldata,odr_hdr->msglen);
	}

	*bufflen=plen;
	//displaypkt(tptr);
	}
	return tptr;
}

void sendPLP(routeInfo *rinfo,pkt *p,struct sockaddr_ll *rcv_sa,int sockfd)
{
	int bufflen=0;
    struct sockaddr_ll sa;
   	void *ptr = framepacket(rinfo,&p->odrInfo,p->data,&bufflen,&sa,RREP,NULL,0);

	if(ptr)
	{
    	int len = sizeof(struct sockaddr_ll);
    	int nbytes = sendto(sockfd, ptr, bufflen, 0, (struct sockaddr *)&sa, len);
    	if(nbytes < 0)
    	{
        	perror("sendto");
    	}
    	printf("HDLR : Sent %d bytes \n",nbytes);
	}
	else
		printf("HDLR : framepacket returned NULL \n");
}

void sendRREP(routeInfo *rinfo,pkt *p,struct sockaddr_ll *rcv_sa,int sockfd,int flip)
{

	int bufflen=0;
	struct sockaddr_ll sa;
	
	
	void *ptr = framepacket(rinfo,&p->odrInfo,p->data,&bufflen,&sa,RREP,NULL,flip);
	int len = sizeof(struct sockaddr_ll);
	int nbytes = sendto(sockfd, ptr, bufflen, 0, (struct sockaddr *)&sa, len);
	if(nbytes < 0)
	{
		perror("sendto");
	}
	printf("HDLR : Sent %d bytes \n",nbytes);

}

void forwardPLP(routeInfo *rinfo, pkt *p, int sockfd)
{
  int bufflen = 0;
  struct sockaddr_ll sa;
  void *ptr = framepacket(rinfo, &p->odrInfo, p->data, &bufflen, &sa, PLP, NULL,0);
  socklen_t len = sizeof(struct sockaddr_ll);
  int nbytes = sendto(sockfd, ptr, bufflen, 0, (struct sockaddr *)&sa, len);
  if (nbytes < 0)
    {
      perror("PLP sendto");
    }
  printf("HDLR : Send %d bytes\n", nbytes);
}

void broadcastRREQ(pkt *p,struct sockaddr_ll *rcv_sa,int sockfd)
{
	//not sure how to do this...
	//dest_mac = ff:ff:ff:ff:ff:ff
	//dest_ip = dest_ip
	//src_ip = src_ip
	//hc = hc+1;
	//timestamp = new timestamp
	//brdcastid = brdcastid;

	int bufflen = 0;
	struct sockaddr_ll sa;

	extern int max_interface;
	int i =0;
	routeInfo rinfo;
	
	for(i=0;i<max_interface+1;i++)
	{
		memset(&rinfo,0,sizeof(rinfo));
		rinfo.ifid = i;
		rinfo.hopCount = p->odrInfo.hopcount+1;
		rinfo.broadcastID = 0;
		rinfo.timestamp = 0;
		strcpy(rinfo.dest_ipaddr,getIP(p->odrInfo.dest_addr));
		strcpy(rinfo.nexthop_addr,"ff:ff:ff:ff:ff:ff");
		void *ptr = framepacket(&rinfo,&p->odrInfo,p->data,&bufflen,&sa,RREQ,(struct ether_addr *)&p->ethInfo.h_dest,0);
		if(ptr)
		{
			int len = sizeof(struct sockaddr_ll);
			int nbytes = sendto(sockfd, ptr, bufflen, 0, (struct sockaddr *)&sa, len);
			if(nbytes < 0)
			{
				perror("sendto");
			}
			printf("HDLR : Sent %d bytes \n",nbytes);
		}
	}

}

char * getIP(unsigned int ip)
{
	struct in_addr ipaddr;
	ipaddr.s_addr = ip;
	return (inet_ntoa(ipaddr));
}
