#ifndef _letsdothis_h
#define _letsdothis_h

#include "unpifiplus.h"
#define PSH 101
#define ACK 102
#define FIN 103
#define FIN_ACK 105
#define _EOF 104

#define MAX_DATA_LEN 488
#define MAX_WNDW_SIZE 20
#define SLOW_START 1
#define CONGESTION_AVOIDANCE 2
#define FAST_RETRANSMIT 3
#define FAST_RECOVERY 4
#define MAX_SERVER_WNDW_SIZE 20

#define FIN_WAIT_1 1
#define FIN_WAIT_2 2 
#define TIME_WAIT  3
#define CLOSE_WAIT 4
#define LAST_ACK   5


typedef long long sequence;

typedef struct _peerInfo
{
	int sockfd;
	struct sockaddr saddr;
}peerInfo;

#pragma pack(1)

typedef struct _header
{
	sequence seqNo;
	sequence ackNo;
	int type;
	int msglen;
	int cwndw_size;
	float drop_probablity;
}tcpHeader;

typedef struct _packet
{
	tcpHeader header;
	char data[MAX_DATA_LEN];
}tcpPacket;

typedef struct _window
{
	tcpPacket windElems[MAX_WNDW_SIZE];
	int wndw_size;
	int cwndw_size;
	int LFA;
	int LFR;
	int CRP;
	int CWP;
	int state;
}window;

typedef struct _swindow
{
	tcpPacket windElems[MAX_SERVER_WNDW_SIZE];
	int wndw_size;
	int cwndw_size;
	int LFA;
	int LFS;
	int CRP;
	int CWP;
	int state;
}swindow;

typedef struct _printThreadArgs
{
	pthread_mutex_t mutex;
	pthread_cond_t cond;
}printThreadArgs;

typedef struct _recvThreadArgs
{
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	peerInfo peer;
}recvThreadArgs;

	
/**************** FUNCTION DECLARATIONS **********************/


void reliableRecvUDP(void *args);
int readData(peerInfo peer,void *buff,int bufflen,int flags);
void printData(void *args);
int creatThreads(int sockfd, struct sockaddr sa);
//int reliableSendUDP(int sockfd, void *buffer, int n_bytes,int advertised_wndw_size);
int reliableSendUDP(peerInfo peer,void *buffer,int n_bytes,int last_packet);
int sendData(peerInfo peer, void *buffer, int buff_len,int flags);
int calc_timeout();
int sort_packets(int *);
int drop_packets();
void getAcks(peerInfo);
void readFINData(peerInfo,tcpHeader *);
void activeClose(peerInfo);
void passiveClose(peerInfo);


#endif
