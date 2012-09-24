#include "letsdothis.h"
	
window client_wndw;
#define DROP_THRESHOLD 0.50000
FILE *file;
int file_transfer_status;

void init_client_wndw()
{
	memset(&client_wndw,0,sizeof(client_wndw));
	client_wndw.LFA = 0;
	client_wndw.LFR = 0;
	client_wndw.CWP = 0;
	client_wndw.CRP = -1;
	client_wndw.wndw_size = 10;
	client_wndw.cwndw_size = 10;
	file = fopen("file_transfer.tmp","w");
	//if(file)
		//printf("TMP FILE FOR FILE TRANSFER HAS BEEN CREATED...\n");
}
void reliableRecvUDP(void *args)
{

/* static LFR = 0, LFA = 0; 
*  this function when called, will send back UPTO n_bytes of data to the calling program. 
*  it maintains a window of size m msgs, and keeps filling it as the server sends the datagram. 
*  each msg in the window can be of size x bytes.
*  if the requested size n_bytes is greater than the size of data that can be stored in the window, then the function will return m*x bytes. So the function has to be called in a while loop till we receive all the data. 
*  peek into the msgs from the socket, and find the LFR in order and send an ACK to the server. 
*  now read the data and add msgs with seqno in the range LFR - LFA, discard rest of the data. 
*  Keep looping till we fill up the whole window or we read n_bytes.
*  if the connection gets reset in the mid way return -1 
*  else return bytes_read; 
*  reliableRecvUDP will make sure the returned bytes are in order!! - FLOW CONTROL. 
*  reliableRecvUDP will also send the ACK packets to the server. 
*/

	recvThreadArgs *threadArgs = (recvThreadArgs *)args;

	pthread_mutex_t *mtx;
	pthread_cond_t *cnd;
	peerInfo peer;
	fd_set rset;
	int maxfdp;
	int rc;

	int flags;
	int bytes_recvd,packets_recvd,packets_drpd;



	//memcpy(&mtx,&threadArgs->mutex,sizeof(pthread_mutex_t));
	//memcpy(&cnd,&threadArgs->cond,sizeof(pthread_cond_t));
	
	mtx = &threadArgs->mutex;
	cnd = &threadArgs->cond;

	memcpy(&peer,&threadArgs->peer,sizeof(peerInfo));

	struct sockaddr_in sa;
	memcpy(&sa,&peer.saddr,sizeof(sa));

	//printf("R-RECV : SERVER INFO \n");
	//printf("\tSERV SOCK FD : %d\n\tSERV PORT  : %d\n",peer.sockfd,ntohs(sa.sin_port));


	//printf("peer.sockfd : %d\n",peer.sockfd);


	tcpPacket tpkt;
		pthread_mutex_lock(mtx);	
	int closed = 1;
		//pthread_mutex_lock(&mtx);	
	do
	{
		FD_ZERO(&rset);
		FD_SET(peer.sockfd,&rset);
		maxfdp = peer.sockfd + 1;
		//printf("R-RECV : Waiting in select\n");
		rc = select(maxfdp,&rset,NULL,NULL,NULL);

		//printf("R-RECV : SELECT RETURNED : %d\n",rc);

		if(rc == 0)
			printf("TIME OUT\n");
		else if (rc < 0)
		{
			printf("ERROR : BREAKING OUT\n");
			break;
		}
		else
		{
			if(FD_ISSET(peer.sockfd,&rset))
			{
				//peek in the mesage... 
				flags = 0;
				//printf("R-RECV : GOING TO READ DATA\n");
				//printf("\tCURRENT WINDOW WRITE POINTER : %d\n",client_wndw.CWP);
				//printf("\tCURRENT WINDOW READ POINTER  : %d\n",client_wndw.CRP);
				if(client_wndw.CWP <= client_wndw.wndw_size)
				{

					//bytes_recvd = readData(peer,client_wndw.windElems[client_wndw.CWP],sizeof(tcpPacket),flags);
					bytes_recvd = readData(peer,&tpkt,sizeof(tcpPacket),flags);

					//if(tcpPacket.header.seqNo == client_wndw.LFR+1)	

					memcpy((client_wndw.windElems+client_wndw.CWP),&tpkt,sizeof(tcpPacket));
					//printf("%d, %lld\n",client_wndw.CWP,tpkt.header.seqNo);
					
					client_wndw.CWP++;
					//printf("R-RECV : Data recvd from server of size : %d\n",bytes_recvd);
					printf("RECVD : P-SEQ_NO  : %lld",client_wndw.windElems[client_wndw.CWP-1].header.seqNo);
					//printf("\tP-MSG_LEN : %d",client_wndw.windElems[client_wndw.CWP-1].header.msglen);
					//printf("\tP-MSG_TYPE: %d\n",client_wndw.windElems[client_wndw.CWP-1].header.type);
					file_transfer_status = client_wndw.windElems[client_wndw.CWP-1].header.type;


					client_wndw.LFR = client_wndw.windElems[client_wndw.CWP-1].header.seqNo;
					client_wndw.LFA = client_wndw.LFR+1;
					client_wndw.cwndw_size = client_wndw.wndw_size - client_wndw.CWP;
					tcpHeader ackPacket;
					ackPacket.seqNo = client_wndw.LFA-1;
					ackPacket.ackNo = client_wndw.LFA;
					ackPacket.type = ACK;
					ackPacket.msglen = 0;
					ackPacket.cwndw_size = client_wndw.cwndw_size-1;
					flags = 0;
					if ( file_transfer_status == 104)
						client_wndw.CWP = client_wndw.wndw_size;
					if(client_wndw.CWP == client_wndw.wndw_size)
					{
					#if 1
						pthread_cond_broadcast(cnd);
						//pthread_cond_signal(cnd);
		        			pthread_mutex_unlock(mtx);

						printf("R_READ : SIGNALLED TO R_PRINT\n");
						sleep(3);	
						pthread_mutex_lock(mtx);
						printf("R_READ : GOT THE CONTROL BACK\n");

						while(client_wndw.CRP==0)
							pthread_cond_wait(cnd,mtx);

					#endif
						//send an ack here...
						client_wndw.CWP = 0;
						client_wndw.CRP = -1;
                                       		ackPacket.cwndw_size = 10;
                                       		flags = 0;
						//printf("after reseting client_wndw.CRP : %d\n",client_wndw.CRP);
					}
					printf("\tSENT : P-ACK_NO : %lld\n",ackPacket.ackNo);
					sendData(peer,&ackPacket,sizeof(tcpHeader),flags);
					
						
				}
				if(file_transfer_status == 104)
				{
					printf("-----------------------PASSIVE CLOSE---------------------\n");
					passiveClose(peer);
					file_transfer_status = 1;
				}
				
			}
		}


	}while(file_transfer_status!=closed);

	printf("FILE TRANSFER COMPLETE...\n");
	//pthread_mutex_unlock(&mtx);
	pthread_mutex_unlock(mtx);
	//pthread_mutex_lock(&mtx);
	//pthread_cond_wait(&cnd,&mtx);
	
	


}

void passiveClose(peerInfo peer)
{
        tcpPacket fin1Pkt;
        tcpHeader fin1ack;
        tcpHeader fin2Pkt;
        tcpPacket fin2ack;

	while(client_wndw.state != CLOSE_WAIT)	
	{
		fin1ack.type = FIN_ACK;
		fin1ack.msglen = 0;
		fin1ack.seqNo = 0;
		fin1ack.ackNo = 0;
		fin1ack.drop_probablity = 1.0;
		readPackets(peer,&fin1Pkt,sizeof(tcpPacket),0);

		if(fin1Pkt.header.type == FIN)
		{
		printf("<--FIN RECVD\n");
		sendData(peer,&fin1ack,sizeof(tcpHeader),0);	
		printf("FIN ACK SENT-->\n");
		client_wndw.state = CLOSE_WAIT;
		}
	}

	fin2Pkt.type = FIN;
	fin2Pkt.msglen = 0;
	fin2Pkt.seqNo = 0;
	fin2Pkt.ackNo = 0;
	fin2Pkt.drop_probablity = 1.0;
	printf("FIN 2 SENT-->\n");
	sendData(peer,&fin2Pkt,sizeof(tcpHeader),0);	

	while ( client_wndw.state != LAST_ACK)
	{
		readPackets(peer,&fin2ack,sizeof(tcpPacket),0);
		if(fin2ack.header.type == FIN_ACK)
		{
			printf("<--FIN ACK RECVD\n");
			client_wndw.state = LAST_ACK;
		}

	}
	printf("COMMUNICATION CLOSED PROPERLY\n");


}

int readPackets(peerInfo peer,void *buff,int bufflen,int flags)
{
	int bytes_read;
	int sa_len = sizeof(struct sockaddr);

	bytes_read = recvfrom(peer.sockfd,buff,bufflen,flags,&peer.saddr,&sa_len);

	return bytes_read;
}

void printData(void *args)
{
	printThreadArgs *threadArgs = (printThreadArgs *)args;
	
	pthread_mutex_t *mtx;
	pthread_cond_t *cnd;

	//pthread_cond_t cnd;
	//pthread_mutex_t mtx;

        //memcpy(&mtx,&threadArgs->mutex,sizeof(pthread_mutex_t));
        //memcpy(&cnd,&threadArgs->cond,sizeof(pthread_cond_t));

	mtx = &threadArgs->mutex;
	cnd = &threadArgs->cond;
	
	int empty,eof=0;

	do
	{
		printf("R_PRINT : WAITING FOR THE LOCK\n");
       		pthread_mutex_lock(mtx);
		printf("R_PRINT : WAITING FOR THE COND\n");
		while(client_wndw.CWP<client_wndw.wndw_size)
		pthread_cond_wait(cnd,mtx);


		if(client_wndw.CRP == -1 && client_wndw.CWP>0)
		{
			printf("R_PRINT : GOT THE LOCK\n");
			printf("client_wndw.CRP : %d\n",client_wndw.CRP);
			printf("client_wndw.CWP : %d\n",client_wndw.CWP);

			while(client_wndw.CRP != client_wndw.CWP-1)
			{
				client_wndw.CRP = (client_wndw.CRP + 1);
				fprintf(file,"%s",client_wndw.windElems[client_wndw.CRP].data);
				fputs(client_wndw.windElems[client_wndw.CRP].data,stdout);
				if(client_wndw.windElems[client_wndw.CRP].header.type == 104)
				{
					eof = 1;
					fclose(file);
					break;
	
				}
	
			}	
		}

		if( (client_wndw.CRP == client_wndw.CWP-1) || (eof==1))
		{
			client_wndw.CRP=client_wndw.CWP-1;
			printf("SIGNALLED BACK TO R_READ\n");
			pthread_cond_broadcast(cnd);
		}

       		pthread_mutex_unlock(mtx);

		sleep(1);

	}while(eof==0);
       	//pthread_mutex_unlock(mtx);
	
}





int createThreads(int sockfd, struct sockaddr * sa)
{
	pthread_mutex_t mutex;
	pthread_cond_t cond;

	pthread_t recv_tid;
	pthread_t print_tid;

	pthread_cond_init(&cond, NULL);
        pthread_mutex_init(&mutex, NULL);

	peerInfo peer;

	peer.sockfd = sockfd;
	memcpy(&peer.saddr,sa,sizeof(struct sockaddr));

	printThreadArgs ptArgs;

	memcpy(&ptArgs.mutex,&mutex,sizeof(pthread_mutex_t));
	memcpy(&ptArgs.cond,&cond,sizeof(pthread_cond_t));

	recvThreadArgs rtArgs;
	memcpy(&rtArgs.mutex,&mutex,sizeof(pthread_mutex_t));
	memcpy(&rtArgs.cond,&cond,sizeof(pthread_cond_t));
	memcpy(&rtArgs.peer,&peer,sizeof(peerInfo));


	if(pthread_create(&recv_tid, NULL, reliableRecvUDP, (void*)&rtArgs) != 0)
	{
                pthread_mutex_destroy(&mutex);
                pthread_cond_destroy(&cond);
                return(EXIT_FAILURE);
        }

        if(pthread_create(&print_tid, NULL, printData, (void*)&ptArgs) != 0)
	{
		printf("printData EXITS\n");
                pthread_mutex_lock(&mutex);
                pthread_mutex_unlock(&mutex);
                pthread_cond_signal(&cond);
                pthread_join(recv_tid, NULL);
                pthread_mutex_destroy(&mutex);
                pthread_cond_destroy(&cond);;
                return(EXIT_FAILURE);
        }

        pthread_join(recv_tid, NULL);
        pthread_join(print_tid, NULL);

        pthread_mutex_destroy(&mutex);
        pthread_cond_destroy(&cond);

        return(EXIT_SUCCESS);

	
}


int sort_packets(int *ftcomp)
{
	int i,j;
	for (i=0;i<client_wndw.wndw_size;i++)
	{
		for(j=0;j<client_wndw.wndw_size;i++)
		{
			if(client_wndw.windElems[i].header.seqNo > client_wndw.windElems[j].header.seqNo) 
			{
				tcpPacket temp;
				memcpy(&temp,(client_wndw.windElems+i),sizeof(tcpPacket));
				memcpy((client_wndw.windElems+i),(client_wndw.windElems+j),sizeof(tcpPacket));
				memcpy((client_wndw.windElems+j),&temp,sizeof(tcpPacket));
			}
		}
	}
	int ackNo = client_wndw.windElems[client_wndw.wndw_size-1].header.seqNo+1;
	int ackIndex = client_wndw.wndw_size;
	for (i=0;i<client_wndw.wndw_size-1;i++)
	{
		if(client_wndw.windElems[i].header.seqNo != client_wndw.windElems[i+1].header.seqNo-1)
		{
			ackNo = client_wndw.windElems[i].header.seqNo+1;
			ackIndex = i;

			
		}

	}
	
	*ftcomp =  0;
	for(i=0;i<ackIndex;i++)
		if(client_wndw.windElems[i].header.type == _EOF)
			*ftcomp = 1;

	for(j=ackIndex+1;j<client_wndw.wndw_size;j++)
		memset((client_wndw.windElems+i),0,sizeof(tcpPacket));

	return ackNo;

}

int drop_packets()
{
	int i=0,j=0;
	int next=1;

	int packets_dropped = 0	;

	do
	{
		if(client_wndw.windElems[i].header.drop_probablity < DROP_THRESHOLD )
		{
			memset((client_wndw.windElems+i),0,sizeof(tcpPacket));
                        memcpy((client_wndw.windElems+i),(client_wndw.windElems+next),sizeof(tcpPacket));

			next++;
			packets_dropped++;

		}
		else
		{
			i++;
			if(i!=next)
                        	memcpy((client_wndw.windElems+i),(client_wndw.windElems+next),sizeof(tcpPacket));
			next++;
			
		}
		

	}while(next<=client_wndw.wndw_size);

	return packets_dropped;

}

int sendData(peerInfo peer, void *buffer, int buff_len,int flags)
{
        int bytes_sent ;
	int sa_len;
	sa_len = sizeof(struct sockaddr);
	//printf("R-RECCV : SENDING ACK TO SOCKFD : %d\n",peer.sockfd);
        if(peer.sockfd > 2)
                bytes_sent = sendto(peer.sockfd, buffer, buff_len, flags, &peer.saddr,sa_len);
	//printf("BYTES SENT : %d\n",bytes_sent);
	return bytes_sent;
}
int readData(peerInfo peer, void *buffer, int buff_len,int flags)
{
        int bytes_recvd ;
	tcpPacket data;
	int sa_len;
	sa_len = sizeof(struct sockaddr);
	//printf("R-RECCV : READING DATA FROM SOCKFD : %d\n",peer.sockfd);

	memset(&data,0,sizeof(data));
        if(peer.sockfd > 2)
                bytes_recvd = recvfrom(peer.sockfd, &data, buff_len, flags, &peer.saddr,&sa_len);

#if 1
	//fputs(data.data,file);
#endif
	//printf("BYTES READ  : %d \n",bytes_recvd);
	memcpy(buffer,&data,sizeof(data));
	return bytes_recvd;
}


