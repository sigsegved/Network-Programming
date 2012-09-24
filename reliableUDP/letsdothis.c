
#include "letsdothis.h"


swindow server_wndw;

void init_server_window(int m)
{

#if 0	
	if(rttinit == 0)
	{
		rtt_init(&rttinfo);
		rttinit = 1;
		rtt_d_flag = 1;	
	}
#endif


	memset(&server_wndw,0,sizeof(swindow));
	server_wndw.CRP = -1;
	server_wndw.CWP = 0;
	server_wndw.LFA = 1;
	server_wndw.LFS = 0;
	server_wndw.cwndw_size = 10;
	server_wndw.wndw_size = MAX_SERVER_WNDW_SIZE;
}

int reliableSendUDP(peerInfo peer,void *buffer,int n_bytes,int last_packet)
{
	int i,j,k;
	static int seq_no;
	tcpPacket cpkt;

	/*
	 * peerInfo peer;
	memset(&peer,0,sizeof(peer));
	peer.sockfd = sockfd;
	memcpy(&peer.saddr,client,sizeof(struct sockaddr));

	 */

	//constructing the packet to be sent....
	//only if the sender window is not full...
	int window_full = 1;
	if(server_wndw.CRP != server_wndw.CWP && server_wndw.CWP<server_wndw.cwndw_size)
		window_full = 0;
	/*
	if(server_wndw.CRP == server_wndw.CWP)
	{
		printf("WINDOW IS FULL\n");
		window_full = 1;
	}	
	*/


	if(window_full)
	{
		getAcks(peer);
		if(server_wndw.CRP != server_wndw.CWP && server_wndw.CWP<server_wndw.cwndw_size)
			window_full = 0;
		else
			window_full = 1;
	}
	if(!window_full)
	{
		//printf("R-SEND : Good to send a packet\n");
		server_wndw.LFS = server_wndw.LFS + 1;
		//printf("R-SEND : INCREMENTING LFS TO : %d\n",server_wndw.LFS);
		memset(&cpkt,0,sizeof(tcpPacket));
		cpkt.header.seqNo = server_wndw.LFS;
		cpkt.header.drop_probablity = 1.0;
		cpkt.header.msglen = n_bytes;
		if(last_packet)
		{
			cpkt.header.type = _EOF;
			printf("INFO : FINAL PACKET P-SEQ_NO : %lld\n",server_wndw.LFS);
		}
		else
			cpkt.header.type = PSH;

		memcpy(&cpkt.data,buffer,n_bytes);

		//add it to the sender window....
		memcpy(&server_wndw.windElems[server_wndw.CWP].header,&cpkt.header,sizeof(tcpHeader));
		memcpy(&server_wndw.windElems[server_wndw.CWP].data,&cpkt.data,n_bytes);
		server_wndw.CWP = (server_wndw.CWP+1) % server_wndw.wndw_size;

		//send the packet
		//printf("R-SEND : PACKET CONSTRUCTED...\n");
		printf("SENT : P-SEQ_NO  : %lld\n",cpkt.header.seqNo);
		//printf("\tW-CWP : %d\t W-CRP : %d\n",server_wndw.CWP,server_wndw.CRP);
		//printf("\tP-MSG_LEN : %d\n",cpkt.header.msglen);
		sendData(peer,&cpkt,sizeof(tcpPacket),0);
		if(last_packet)
		{
			printf("LAST PACKET SENT \n");
			//printf("server_wndw.LFS : %d\n",server_wndw.LFS);
			//printf("EXPECTED LFA : %d\n",server_wndw.LFS+1);
			while(server_wndw.LFA == server_wndw.LFS+1)
			{
				getAcks(peer);
				//printf("server_wndw.LFS : %d\n",server_wndw.LFS);
				//printf("LFA : %d\n",server_wndw.LFA);
				
			}
			activeClose(peer);
		}
	}

}

void activeClose(peerInfo peer)
{
	tcpPacket fin1Pkt;
	tcpHeader fin1ack;
	tcpHeader fin2Pkt;
	tcpPacket fin2ack;
	
	fin1Pkt.header.type = FIN;
	fin1Pkt.header.seqNo = 0;
	fin1Pkt.header.msglen = 0;
	fin1Pkt.header.ackNo = 0;
	fin1Pkt.header.drop_probablity = 1.0;
	printf("---------------- ACTIVE CLOSE -------------------------\n");
	//memcpy(&server_wndw.windElems[server_wndw.CWP].header,&fin1Pkt.header,sizeof(tcpHeader));
	//memcpy(&server_wndw.windElems[server_wndw.CWP].data,&fin1Pkt.data,sizeof(fin1Pkt.data));
	//server_wndw.CWP = (server_wndw.CWP+1) % server_wndw.wndw_size;

		printf("-->FIN SENT\n");
		sendData(peer,&fin1Pkt,sizeof(tcpPacket),0);
        	server_wndw.state = FIN_WAIT_1;
		printf("CURRENT STATE : FIN_WAIT_1\n");
	while(server_wndw.state != FIN_WAIT_2)
	{
		memset(&fin1ack,0,sizeof(fin1ack));
		readFINData(peer,&fin1ack);
		//printf("fin1ack.type : %d\n",fin1ack.type);
		if(fin1ack.type == FIN_ACK)
		{
			printf("<-- FIN ACK RECVD\n");
			server_wndw.state = FIN_WAIT_2;
			server_wndw.LFA = fin1ack.ackNo;
		}
	}
	printf("CURRENT STATE : FIN_WAIT_2\n");
	while(server_wndw.state != TIME_WAIT)
	{
		memset(&fin2Pkt,0,sizeof(fin2Pkt));
		readFINData(peer,&fin2Pkt);
		//printf("fin2Pkt.type : %d\n",fin2Pkt.type);
		if(fin2Pkt.type == FIN)
		{
			printf("<--FIN RECVD\n");
			server_wndw.state = TIME_WAIT;
			fin2Pkt.type = FIN_ACK;
			fin2Pkt.seqNo = 0;
			fin2Pkt.msglen = 0;
			fin2Pkt.ackNo = 0;
			fin2Pkt.drop_probablity = 1.0;
			sendData(peer,&fin2Pkt,sizeof(tcpPacket),0);
			printf("FIN ACK SENT-->\n");
		}
	}
	printf("CURRENT STATE : TIME_WAIT\n");
	close(peer.sockfd);
	printf("CURRENT STATE : CLOSED\n");
	
}

void readFINData(peerInfo peer,tcpHeader *ackPkt)
{
	fd_set rset;
	struct timeval timeout;
	timeout.tv_sec = 2;
	timeout.tv_usec = 0;
	int rc;
	struct sockaddr csa;
	int size_csa;
	int maxfdp;
	
	//do
	//{
		FD_ZERO(&rset);
		FD_SET(peer.sockfd,&rset);
		maxfdp = peer.sockfd+1;
		rc  = select(maxfdp,&rset,NULL,NULL,NULL);
		//printf("FIN READ SELECT RETURNED %d \n",rc);
		size_csa = sizeof(csa);
		memcpy(&csa,&peer.saddr,sizeof(csa));

		if( rc == 0 )
		{
        		printf("FIN ACKS TIMED OUT\n");
        		//break;
        		//RETRANSMIT....
		}
		if( rc < 0)
		{
        		printf("OOPS SERVER DOWN\n");
        		//break;
		}
		else
		{
			int br = recvfrom(peer.sockfd,ackPkt,sizeof(tcpHeader),0,&csa,&size_csa);
		//	printf("FIN READ GOT %d bytes \n",br);
		}
	//}while(1);
}
	
void getAcks(peerInfo peer)
{

	//else
	//if((window_full) || (last_packet))
	{
		/*if(window_full)
			printf("R-SEND : SENDER WINDOW FULL\n");
		else
			printf("R-SEND : LAST PACKET SENT\nR-SEND : WAITING FOR FINAL ACKS\n");
		*/
		printf("INFO : SENDER WINDOW FULL\n");
                printf("---------------------------------------------------\n");
		tcpHeader ackPkt;
		int init_ack_no = server_wndw.LFA;
		struct sockaddr csa;
		int size_csa;
		int ackCount=0;
		int final_ack_count=0;
		fd_set rset;
		int maxfdp;
		struct timeval rtt_tout;
		int rc;

		printf("PHASE : READ ACKS\n");
		do
		{

			FD_ZERO(&rset);
			FD_SET(peer.sockfd,&rset);
			maxfdp = peer.sockfd+1;
			rtt_tout.tv_sec = 6;
			rtt_tout.tv_usec = 0;

			//printf("PEER SOCK FD : %d\n",peer.sockfd);

			rc  = select(maxfdp,&rset,NULL,NULL,&rtt_tout);
			//printf("SELECT RETURNED %d \n",rc);
				
			if( rc == 0 )
			{
				printf("ACKS TIMED OUT\n");
				break;
				//RETRANSMIT....
			}
			if( rc < 0)
			{
				printf("OOPS SERVER DOWN\n");
				break;
			}
			else
			{

			//recv all ACKS!!!!
			//reset the timer if the ACK is in order... -- NOT NOW
			//calculate new RTT...
			size_csa = sizeof(csa);
			memcpy(&csa,&peer.saddr,sizeof(csa));
			//printf("-SEND : Waiting for ACK\n");

			recvfrom(peer.sockfd,&ackPkt,sizeof(ackPkt),0,&csa,&size_csa);

			//printf("-SEND 	: ACK RECVD\n");
			printf("\tRECVD : P-ACK_NO : %lld\n",ackPkt.ackNo);
			//printf("\tWNDW SIZE : %d\n",ackPkt.cwndw_size);

			if(ackPkt.ackNo >= server_wndw.LFA)
			{
				server_wndw.LFA = ackPkt.ackNo;
				server_wndw.cwndw_size = ackPkt.cwndw_size;
			}
			if(server_wndw.LFA == server_wndw.LFS+1)
				final_ack_count++;
			//use select with timeout mechanism
			ackCount++;
			}

		}while(final_ack_count<1);
		printf("\nINFO : Advertised Client Window Size : %d\n",server_wndw.cwndw_size);
		printf("INFO : FINISHED READING ALL ACKS\n");
                printf("---------------------------------------------------\n");

		//ackCount--;

		//printf("ACTUAL ACKS RECVD = %d\n",ackCount);

		ackCount = server_wndw.LFA - init_ack_no ;
		//printf("No. of ACKs recvd : %d\n",ackCount);

		printf("\t PHASE : PROCESS ACKS\n");

		do
		{
			server_wndw.CRP = (server_wndw.CRP+1)% server_wndw.wndw_size;
			//printf("NULLIFYING PACKET NO : %d\n",server_wndw.CRP);
			memset(&server_wndw.windElems[server_wndw.CRP].header,0,sizeof(tcpHeader));
			memset(server_wndw.windElems[server_wndw.CRP].data,0,MAX_DATA_LEN);
			ackCount--;
		}while(ackCount>=0);
		printf("INFO : FINISHED PROCESSING ALL ACKS\n");
                printf("---------------------------------------------------\n");

		int tmp_crp;
		tmp_crp = server_wndw.CRP ;
		tcpPacket tPkt;
		ackCount = server_wndw.LFA - init_ack_no - 1;
		//printf("tmp_crp : %d\n",tmp_crp);
		//printf("server_wndw.CWP : %d\n",server_wndw.CWP);
		printf("PHASE : FAST RETRANSMIT\n");
		while(tmp_crp!=server_wndw.CWP)
		{
			//retransmit packets starting from LFA to LFS.
			tmp_crp = (tmp_crp + 1 )%server_wndw.wndw_size;
			printf("\tP-SEQ_NO : %d\n",tPkt.header.seqNo);
			memcpy(&tPkt.header,&server_wndw.windElems[tmp_crp].header,sizeof(tcpHeader));
			memcpy(&tPkt.data,&server_wndw.windElems[tmp_crp].data,sizeof(tPkt.data));
			sendData(peer,(server_wndw.windElems+tmp_crp),sizeof(tcpPacket),0);

		}
			

		printf("INFO: FINISHED RETRANSMITTING ALL LOST PACKETS\n");
		printf("---------------------------------------------------\n");
		server_wndw.CWP = (server_wndw.CRP+1)%server_wndw.wndw_size;
		//printf("server_wndw.CWP : %d\n",server_wndw.CWP);
		//printf("server_wndw.cwndw_size : %d\n",server_wndw.cwndw_size);
		if(server_wndw.CWP-1 == server_wndw.cwndw_size)
		{
			//printf("WINDOW STARTS FROM BEGINING\n");
			server_wndw.CWP = 0;
			server_wndw.CRP = -1;
		}
	}




}
#if 0
int reliableSendUDP(int sockfd, void *buffer, int n_bytes,int advertised_wndw_size,struct sockaddr *client) 
{

/*  LFA = 0, LFS = 0, m = advertised client window size. 
*   LOOP BEGINS
*   reliableSendUDP fucntion will convert the buffer of n_bytes into (n_bytes/x) . 
*   It will send out a maximum of m msgs starting from LFA, to the client, where m = advertised client window size. 
*   update LFS =  LFA + m
*   Waits for the acknowledgement from the client. 
*   Updates the LFA, client window size - m, 
*   if the connection is reset, then Update LFA = -1
*   the looping continues till (LFA == (n_byte/x)) and (LFA!=-1)
*   return the LFA;
*/


	static int LAR;
	static int LFS;

	tcpPacket packet;
	tcpPacket *sndBuff;


	int t_lfs;
	int n_packets = ceil(n_bytes/MAX_DATA_LEN);
	int m=advertised_wndw_size;
	int reno_slow_start_wndw_size = m/2;	
	int curr_wndw_size = reno_slow_start_wndw_size;
	int remaining_bytes = n_bytes;
	int packet_count;
	fd_set rset;
	int tcpState  = SLOW_START;
	int maxfdp;
	int byte_count =0;
	peerInfo peer;
	
	int rc;
	tcpPacket ackPacket;
	peer.sockfd = sockfd;
	memcpy(&peer.saddr,client,sizeof(struct sockaddr ));

	do
	{
		FD_ZERO(&rset);
		FD_SET(peer.sockfd,&rset);
		maxfdp = sockfd + 1;
		t_lfs = LAR;
		sndBuff = (tcpPacket *)malloc(sizeof(tcpPacket)*curr_wndw_size);
		packet_count = 0;
		byte_count = 0;

		//printf("\nCURR WNDW SIZE : %d",curr_wndw_size);

		//copy curr_wndw_size packets into the buffer.
		do
		{
			packet.header.seqNo = t_lfs;
			packet.header.type = PSH;
			packet.header.msglen = (MAX_DATA_LEN<remaining_bytes)?MAX_DATA_LEN:remaining_bytes;
			memcpy(packet.data,buffer,packet.header.msglen);
			memcpy(sndBuff+byte_count,&packet,sizeof(tcpPacket));
			byte_count = byte_count + packet.header.msglen;
			t_lfs++;
			//printf("\nPACKET %d CONSTRUCTED ",packet_count);
			packet_count ++;
		}while(packet_count < curr_wndw_size);

		//send the buffer to client
		sendData(peer,sndBuff,byte_count,0);
		LFS = t_lfs;

		struct timeval rtt_tout;

		rtt_tout.tv_sec = calc_timeout();
		
		rc = select(maxfdp,&rset,NULL,NULL,NULL);
		//rc = select(maxfdp,&rset,NULL,NULL,&rtt_tout);

		if ( rc == 0 )
		{
			printf("TIMEOUT\n.... NO ACK RECVD!!!\n");
			LFS = LAR;
		}
		else if(rc == -1)
		{
			printf("ERROR!!!!\n");
			break;
		}
		else
		{
			//recv PACKET....
			//if(ackPacket.header.type == ACK)
			{
				LAR = ackPacket.header.ackNo;
				curr_wndw_size = ackPacket.header.cwndw_size;
				
			}
		}

		
		

	}while(1);
}
#endif

int sendData(peerInfo peer, void *buffer, int buff_len,int flags)
{
	int bytes_sent ;	
	struct sockaddr_in t;
	memcpy(&t,&peer.saddr,sizeof(t));
	//printf("\nSENDING DATA TO CLIENT OF LEN :%d ",buff_len);
	//printf(" Client Port : %d\n",ntohs(t.sin_port));
	//printf("lient SOCKFD : %d\n",peer.sockfd);
	bytes_sent = sendto(peer.sockfd, buffer, buff_len, flags, &peer.saddr, sizeof(struct sockaddr));
	//printf("BYTES SENT TO CLIENT :%d Bytes\n\n",bytes_sent);
}

int calc_timeout()
{
	return 30;
}


