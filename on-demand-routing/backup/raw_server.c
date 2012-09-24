#include <asm/types.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h> /* The L2 protocols */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>


int main()
{

	int protocol_num=200;

	int sockfd;

	sockfd = socket(PF_PACKET,SOCK_RAW, protocol_num);

	if(sockfd > 0)
	{
		printf("Socket created successfully : %d\n",sockfd);
	}
	else
		perror("SOCK ERROR ");

	struct sockaddr_ll sa;

	memset(&sa,0,size(sa));
		
	sa.sll_family = AF_PACKET;
	sa.sll_protocol = htons(200);
	sa.sll_ifindex = 0;
	sa.sll_hatype = 1;
	sa.sll_pkttype = 2;
	sa.sll_halen = 6;
	sa.sll_addr[0]=0x00;
	sa.sll_addr[1]=0x22;
	sa.sll_addr[2]=0x19;
	sa.sll_addr[3]=0xf4;
	sa.sll_addr[4]=0xc8;	
	sa.sll_addr[5]=0x63;
	sa.sll_addr[6]=0x00;
	sa.sll_addr[7]=0x00;

	odrHeader odrH;

	//fill the eth header...
	//fill up the odr header... 
	//put it in one buffer and broadcast it..
	// now the question is how to broadcast and when to broacast.. 
		//1. Broadcast if the table is stale... 
		//2. Broadcast if the forceBroadcast bit is set
		//3. Broadcast if a route to the destination is not available...	
	send(sockfd,msg,0);

	
}



