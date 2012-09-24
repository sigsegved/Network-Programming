/*
 * packetQ.h
 *
 *  Created on: 19-Nov-2011
 *      Author: dell
 */

#ifndef PACKETQ_H_
#define PACKETQ_H_

#include "hw_addrs.h"
#include "odr_client.h"
#include "tableDDT.h"

typedef struct ethhdr ethHdr;
#define MAX_DATA_LEN 256

typedef struct _pkt
{
	ethHdr ethInfo;
	odrHeader odrInfo;
	odr_client_com data;
}pkt;

typedef struct _Qpkt
{
        struct _pkt packet;
        struct _Qpkt *nextPkt;
}Qpkt;



pkt * getNext();
void insert_pkt(pkt *new_pkt);
Qpkt * getHead();




#endif /* PACKETQ_H_ */
