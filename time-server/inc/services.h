/*
 * CS533_Server.h
 *
 *  Created on: Sep 22, 2011
 *      Author: ukay
 */

#ifndef SERVICES_H_
#define SERVICES_H_

#include "cs533.h"
#include "utils.h"

void echoService(void *);
void daytimeservice(void *);
void getCurrTime(char *);

#define MAX_BUFF_LEN 1024


#endif 

