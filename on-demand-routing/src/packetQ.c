/*
 * packetQ.c
 *
 *  Created on: 19-Nov-2011
 *      Author: dell
 */

#include "packetQ.h"

Qpkt *Qhead =NULL;

void insert_pkt(pkt *new_pkt)
{
	Qpkt *tnew;

	tnew = (Qpkt *)malloc(sizeof(Qpkt));
	memset(tnew,0,sizeof(Qpkt));
	memcpy(&tnew->packet,new_pkt,sizeof(pkt));

	tnew->nextPkt = NULL;

	if(!Qhead)
		Qhead = tnew;
	else
	{
		tnew->nextPkt=Qhead;
		Qhead = tnew;
	}
}

pkt * getNext()
{
	Qpkt *thead;
	pkt *ret_node=NULL;
	thead = Qhead;

	if(thead)
	{
		ret_node = (pkt *)malloc(sizeof(pkt));
		memset(ret_node,0,sizeof(pkt));
		memcpy(ret_node,&thead->packet,sizeof(pkt));
		Qhead = Qhead->nextPkt;
		free(thead);
	}
	return ret_node;
}

Qpkt * getHead()
{
	return Qhead;
}



