/*
 * CS533_Server.h
 *
 *  Created on: Sep 22, 2011
 *      Author: ukay
 */

#ifndef CS533_SERVER_H_
#define CS533_SERVER_H_

#include "cs533.h"
#include "utils.h"
#include "services.h"

int startSuperServer(int echoSockFD,int daytimeSockFD);
void acceptConnections(int serviceID, int echoSockFD, int daytimeSockFD);
int startServices(int *echoFD, int *dtFD);
int createServerSocket(int port,int *sockfd);





#endif /* CS533_SERVER_H_ */
