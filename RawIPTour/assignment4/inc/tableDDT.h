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

int addrouteInfo(routeInfo *rtinfo);
routeInfo * getrouteInfo(unsigned int destIP);
void destroyRouteInformation(unsigned int destIP);
void deleteTourInformation(int tfd,int);
void destroyList();
int isStale(unsigned int);


#endif /* TABLEDDT_H_ */
