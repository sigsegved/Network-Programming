#include "hw_addrs.h"
#define MAX_TIMEOUT 10

int areq (struct sockaddr *ip_addr, socklen_t sa_len, struct _hwaddr *hw_addr);
char * getIP(unsigned int ip);

#if 0

int main()
{
	struct sockaddr_in ip;
	struct _hwaddr hw;
	char cip[20];
	memset(cip,0,sizeof(cip));

	printf("Enter IP : ");
	scanf("%s",cip);

	memset(&ip,0,sizeof(struct sockaddr_in));
	memset(&hw,0,sizeof(struct _hwaddr));
	if(inet_aton(cip,&ip.sin_addr)<0)
	{
		printf("INVALID IP ADDRESS\n");
		exit(-1);
	}

	int rc = areq((struct sockaddr *)&ip,sizeof(struct sockaddr_in),&hw);


	printf("RC : %d \n",rc);
	return 1;
}
#endif


int areq (struct sockaddr *ip_addr, socklen_t sa_len, struct _hwaddr *hw_addr)
{

	int sockfd = -1;

	sockfd = socket(AF_UNIX,SOCK_STREAM,0);

	if(sockfd == -1)
	{
		perror("ARP-API  : ");
		return -1;
	}

	/*

	struct sockaddr_un cli_name;

        cli_name.sun_family = AF_UNIX;
        strcpy(cli_name.sun_path, "client.XXXXXX");

        int fd = mkstemp(cli_name.sun_path);
        if (fd == -1)
        {
                printf("Unable to create temporary file\n");
        }
        unlink(cli_name.sun_path);
        close(fd);


	if (bind(sockfd, (struct sockaddr *) &cli_name, sizeof(struct sockaddr_un))) {
        	printf("Error binding a datagram socket");
                return -1;
    	}
	*/


	struct sockaddr_un serv_addr;

	memset(&serv_addr,0,sizeof(serv_addr));
	serv_addr.sun_family = AF_UNIX;
	strcpy(serv_addr.sun_path,SOCK_COM_FILE);

	int rc;
	rc  = connect(sockfd, (struct sockaddr *)&serv_addr, SUN_LEN(&serv_addr));
      	if (rc < 0)
      	{
        	perror("connect() failed");
		return -1;
      	}


	struct sockaddr_in ip;
	memset(&ip,0,sizeof(ip));

	memcpy(&ip,ip_addr,sizeof(ip));
	printf("ARP-API : ASKING ARP TO FETCH THE HW_ADDR FOR %s\n",getIP(ip.sin_addr.s_addr));

	//if(sendto(sockfd,(void *)ip_addr,sizeof(struct sockaddr),0,(struct sockaddr *)&serv_addr,sizeof(struct sockaddr_un)) < 0)

	if(send(sockfd,(void *)ip_addr,sizeof(struct sockaddr),0)<0)
	{
		perror("APR-API : ");
		close(sockfd);
		return -1;
	}

	fd_set rset;
	memset(&rset,0,sizeof(fd_set));

	FD_SET(sockfd,&rset);
	int maxfd = sockfd + 1;
	struct timeval time_out;

	memset(&time_out,0,sizeof(struct timeval));
	time_out.tv_sec = MAX_TIMEOUT;

	printf("ARP-API : WAITING FOR ARP TO RESPOND \n");

	rc = select(maxfd,&rset,NULL,NULL,&time_out);
	char buff[1024];


	if(rc == 0)
	{
		printf("ARP-API : Timed out \n");
		close(sockfd);
		return -1;
	}
	else
	{
		memset(buff,0,sizeof(buff));
		printf("ARP-API : RECVD ARP INFO FROM APR MODULE \n");
		//recvfrom(sockfd,buff,1024,0,(struct sockaddr *)&sa,&len);
		if(recv(sockfd,buff,1024,0) < 0)
		{
			perror("RECV ERROR ");
			return -1;
		}
		memcpy(hw_addr,buff,sizeof(struct _hwaddr));

		printf("HW ADDR : %s\n",ether_ntoa((struct ether_addr *)&hw_addr->sll_addr));
		close(sockfd);
	}

	return 1;
}	
char * getIP(unsigned int ip)
{
        struct in_addr ipaddr;
        ipaddr.s_addr = ip;
        return (inet_ntoa(ipaddr));
}

