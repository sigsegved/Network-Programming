#ifndef __HW_FUNC_H
#define __HW_FUNC_H

#include "hw_addrs.h"

struct hwa_info *get_hw_addrs();
struct hwa_info *Get_hw_addrs();
void free_hwa_info(struct hwa_info *);
void PrintAddrs(struct hwa_info *head);
int getInterfaceInfo(int,unsigned char *,char *,struct hwa_info *);


#endif
