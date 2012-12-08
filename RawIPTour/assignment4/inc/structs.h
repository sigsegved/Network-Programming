#ifndef STRUCTS
#define STRUCTS

#include <netinet/ip.h>


typedef struct ethhdr ethHdr;

typedef struct {
	char* ip;
	unsigned char hwaddr[6];
} vm_addr;

struct icmpheader {
 unsigned char icmp_type;
 unsigned char icmp_code;
 unsigned short int icmp_cksum;
 /* The following data structures are ICMP type specific */
 unsigned short int icmp_id;
 unsigned short int icmp_seq;
 
 unsigned char icmp_data[64];
}; /* total icmp header length: 8 bytes (=64 bits) */


#define MAX_TOUR_SIZE 50
#define TOUR_PROTOCOL_ID 66
typedef struct {
	struct ip iph;
	unsigned int multicast_ip;
	unsigned int multicast_port;
	
	unsigned int source_ip;
	unsigned int current_tour;
	unsigned int tour_ip[MAX_TOUR_SIZE];
} tourpkt;


typedef struct {
	ethHdr ethInfo;
	struct ip iph;
	struct icmpheader icmph;
}  __attribute__((packed)) echopkt;

#endif
