#include "../inc/hw_addrs.h"
#include "../inc/hw_func.h"
#include "../inc/structs.h"
#include "time.h"

#define MULTICAST_IP "225.0.0.37"
#define MULTICAST_PORT 41235

vm_addr vm[10];
int vm_me;
int sock_tour, sock_icmp_req, sock_icmp_resp, sock_multicast;
int is_source;
int visited;
struct hwa_info* eth0;
int ping_active, ping_confirmed, ping_lastnode;
unsigned int ping_addr;
struct sockaddr_in multicast_addr;
struct ip_mreq mreq;
unsigned int multicast_ip, multicast_port;
time_t current_time, shutdown_time;





// Figures out what VM we are by looking at the interfaces
// and also initialized the vm's
void init_vm_recognition() {
	int i;
	struct in_addr comp_addr;
	struct hwa_info *hwahead = NULL, *hwa;
	
	vm[0].ip = "192.168.1.101";
	vm[1].ip = "192.168.1.102";
	vm[2].ip = "192.168.1.103";
	vm[3].ip = "192.168.1.104";
	vm[4].ip = "192.168.1.105";
	vm[5].ip = "192.168.1.106";
	vm[6].ip = "192.168.1.107";
	vm[7].ip = "192.168.1.108";
	vm[8].ip = "192.168.1.109";
	vm[9].ip = "192.168.1.110";

	struct hostent *he;
	struct sockaddr_in server[10];
	char vm_name[20];

	for(i=1;i<=10;i++)
	{
		memset(vm_name,0,sizeof(vm_name));
		sprintf(vm_name,"%s%d","vm",i);
		
		if ((he = gethostbyname(vm_name)) == NULL) {
    		puts("\n** Error resolving hostname **\n");
    		exit(-1);
		}
  		memcpy(&server[i-1].sin_addr, he->h_addr_list[0], he->h_length);
		printf("IP ADDR OF VM%d : %s \n",i,inet_ntoa(server[i-1].sin_addr));
	}

	hwahead = get_hw_addrs();
	if (hwahead == NULL)
	{
		printf("Tour: hwahead = NULL\n");
		exit(1);
	}
	
	vm_me = -1;
	for (hwa = hwahead; hwa != NULL; hwa = hwa->hwa_next) 
	{
		if (strcmp(hwa->if_name, "eth0") == 0)
		{
			eth0 = hwa;
			for (i = 0; i < 10; ++i)
			{
				inet_pton(AF_INET, vm[i].ip, &comp_addr); 
				if (((struct sockaddr_in *)hwa->ip_addr)->sin_addr.s_addr == comp_addr.s_addr)
				{
					vm_me = i;
					break;
				}
			}
			break;
		}
	}
}

// Joins a multicast group, also stores the address for send_multicast to use
void join_multicast(unsigned int addr, unsigned int port)
{
	multicast_ip = addr;
	multicast_port = port;
	
	memset(&multicast_addr, 0, sizeof(multicast_addr));
    multicast_addr.sin_family = AF_INET;
    multicast_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    multicast_addr.sin_port = htons(port);
	 
	 
	if (bind(sock_multicast, (struct sockaddr *) &multicast_addr, sizeof(multicast_addr)) < 0) {
		perror("bind");
		exit(1);
	}
	
	
	mreq.imr_multiaddr.s_addr = addr;
	mreq.imr_interface.s_addr = htonl(INADDR_ANY);
	
	if (setsockopt(sock_multicast, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
		perror("setsockopt");
		exit(1);
	}
	printf("Joined multicast group.\n");
}

// Sends a message to the address that we've joined
void send_multicast(char *msg) {
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = multicast_ip;
	addr.sin_port = htons(multicast_port);
	
	printf("Node vm%d. Sending: %s\n", vm_me + 1, msg);
	
	if (sendto(sock_multicast, msg, strlen(msg) + 1, 0, (struct sockaddr *)&addr, sizeof(addr)) < 0)
	{
		printf("send_multicast failed");
		exit(1);
	}
}

// Received a multicast message, print it out.
void handle_multicast()
{
	char buf[500];
	int addrlen;
	addrlen = sizeof(multicast_addr);
	int bytes_received = recvfrom(sock_multicast, buf, 1000, 0, (struct sockaddr *)&multicast_addr, &addrlen);
	
	if (bytes_received < 0) {
		printf("multicast receive failed\n");
		exit(1);
	}
	
	printf("Node vm%d. Received: %s\n", vm_me + 1, buf);
	if (strstr(buf, "Group members please identify yourselves.") != NULL)
	{
		ping_active = 0;
		
		sprintf(buf, "<<<<< Node vm%d, I am a member of the group. >>>>>", vm_me + 1);
		send_multicast(buf);
		shutdown_time = current_time + 5;
	}
}

// Get the VM number given a string 'vm1'
int get_vm_number(char* label) {
	if (label[0] != 'v' || label[1] != 'm') {
		return -1;
	}
	
	int num = 0;
	label += 2;
	
	while (*label != 0) {
		if (*label >= '0' && *label <= '9')
		{
			num = (num * 10) + (*label - '0');
		}
		else
		{
			return -1;
		}
		label++;
	}
	return num - 1;
}

// Creates the checksum for packets
unsigned short csum(unsigned short *buf,int nwords)
{
	unsigned long sum;
	for (sum = 0; nwords > 0; nwords--){sum += *buf++;}
	sum = (sum >> 16) + (sum & 0xffff);
	sum += (sum >> 16);
	return (unsigned short) (~sum);
}

// Creates the checksum but supports wrong size
uint16_t in_cksum(uint16_t *addr, unsigned len)
{
	uint16_t answer = 0;
	uint32_t sum = 0;
	while (len > 1)  {
		sum += *addr++;
		len -= 2;
	}
	if (len == 1) {
		*(unsigned char *)&answer = *(unsigned char *)addr ;
		sum += answer;
	}
	sum = (sum >> 16) + (sum & 0xffff);
	sum += (sum >> 16);
	answer = ~sum;
	return answer;
}

// Builds the tourpackets IP header and sends it out
void send_tourpacket(tourpkt *tour, unsigned int dst)
{
	tour->multicast_ip = inet_addr(MULTICAST_IP);
	tour->multicast_port = MULTICAST_PORT;

	
	tour->iph.ip_hl = 5;
	tour->iph.ip_v = 4;
	tour->iph.ip_tos = 0;
	tour->iph.ip_len = sizeof(tourpkt);
	tour->iph.ip_id = TOUR_PROTOCOL_ID;
	tour->iph.ip_off = 0;
	tour->iph.ip_ttl = 1;
	tour->iph.ip_p = TOUR_PROTOCOL_ID;
	tour->iph.ip_sum = 0;
	
	tour->iph.ip_src.s_addr = inet_addr(vm[vm_me].ip);
	tour->iph.ip_dst.s_addr = dst;
	tour->iph.ip_sum = csum((unsigned short *)tour, tour->iph.ip_len >> 1);
	

	struct sockaddr_in sin;
	sin.sin_family = PF_INET;
	sin.sin_addr.s_addr = dst;
	//sin.sin_port = htons(55000);
	
	if (sendto(sock_tour, tour, tour->iph.ip_len, 0, (struct sockaddr *)&sin, sizeof(sin)) < 0)
	{
		printf("error in send tour\n");
		exit(1);
	}
	else
	{
		//printf("sent tour\n");
	}
}

// Build the actual tour packet from arguments from user
void build_tour(int length, char* vms[]) {
	int i, vm_index;
	char datagram[4096];
	tourpkt *tour = (tourpkt*)datagram;
	memset(datagram, 0, 4096);

	for (i = 0; (i < length); i++)
	{
		vm_index = get_vm_number(vms[i]);
		if (vm_index == -1)
		{
			printf("Invalid VM number\n");
			exit(1); 
		}
		
		if (vm_me == vm_index) {
			printf("Source node cannot be part of tour!\n");
			exit(1);
		}
		tour->tour_ip[i] = inet_addr(vm[vm_index].ip);
	}
	tour->tour_ip[i] = 0;	
	
	tour->current_tour = 0;
	tour->source_ip = inet_addr(vm[vm_me].ip);
	send_tourpacket(tour, tour->tour_ip[0]);
}

// Find the index of the VM given the IP address
int get_vm_index(unsigned int ipaddr) {
	int i;
	for (i = 0; (i < 10); i++) {
		if (ipaddr == inet_addr(vm[i].ip)) {
			return i;
		}
	}
	return -1;
}

// Sends a ICMP request, uses the ARP project to get the
// mac address first.
void send_icmp_echo_request(unsigned int ipaddr)
{
	echopkt pkt;

	struct _hwaddr hw;
	struct sockaddr_in ip;

	memset(&ip,0,sizeof(ip));
	memset(&hw,0,sizeof(hw));
	ip.sin_addr.s_addr=ipaddr;
	areq(&ip,sizeof(struct sockaddr_in),&hw);
	memcpy(pkt.ethInfo.h_dest,hw.sll_addr,6);
	
	memset(&ip,0,sizeof(ip));
	memset(&hw,0,sizeof(hw));
	inet_aton(vm[vm_me].ip,&ip.sin_addr);
	//ip.sin_addr.s_addr=inet_addr(vm[vm_me].ipaddr);
	areq(&ip,sizeof(struct sockaddr_in),&hw);
	memcpy(pkt.ethInfo.h_source,hw.sll_addr,6);

	pkt.ethInfo.h_proto = htons(ETH_P_IP);
	
	pkt.iph.ip_hl = 5;
	pkt.iph.ip_v = 4;
	pkt.iph.ip_tos = 0;
	pkt.iph.ip_len = htons(sizeof(struct icmpheader) + sizeof(struct ip));
	pkt.iph.ip_id = 0;
	pkt.iph.ip_off = 0;
	pkt.iph.ip_ttl = 64;
	pkt.iph.ip_p = 1;
	pkt.iph.ip_sum = 0;
	
	pkt.iph.ip_src.s_addr = inet_addr(vm[vm_me].ip);
	pkt.iph.ip_dst.s_addr = ipaddr;
	
	pkt.icmph.icmp_type = 8;
	pkt.icmph.icmp_code = 0;
	pkt.icmph.icmp_cksum = 0;
	pkt.icmph.icmp_id = 0x0100;
	pkt.icmph.icmp_seq = 1;
	 
	pkt.icmph.icmp_cksum = in_cksum((unsigned short *)&pkt.icmph, sizeof(struct icmpheader));
	pkt.iph.ip_sum = in_cksum((unsigned short *)&pkt.iph, sizeof(struct icmpheader) + sizeof(struct ip));
	
	struct sockaddr_ll sa;
	sa.sll_family = PF_PACKET;
	sa.sll_protocol = htons(ETH_P_IP); 
	sa.sll_ifindex = eth0->if_index;
	sa.sll_hatype = 0; 
	sa.sll_pkttype = 0; 
	sa.sll_halen = 6;
	memcpy(sa.sll_addr, pkt.ethInfo.h_dest, 6); 
	
	if (sendto(sock_icmp_req, &pkt, sizeof(echopkt), 0, (struct sockaddr *)&sa, sizeof(sa)) < 0)
	{
		printf("error in icmp request\n");
		exit(1);
	}
	else
	{
		//printf("sent icmp request\n");
	}
}

// Receive a tour packet, send it to someone else if im not the last person.
// Also start pinging!
void handle_tourpacket() 
{
	char buf[sizeof(tourpkt)];
	int bytes_received;
	
	bytes_received = recv(sock_tour, buf, sizeof(tourpkt), 0);
	
	if (bytes_received != sizeof(tourpkt))
	{
		printf("Tour Packet size wrong!!\n");
		exit(1);
	}
	
	tourpkt *tour = (tourpkt *)&buf;
	if (tour->iph.ip_id == TOUR_PROTOCOL_ID) {
		printf("%.24s received source routing packet from vm%d\n", ctime(&current_time), get_vm_index(tour->iph.ip_src.s_addr) + 1);
		
		if (visited == 0)
		{
			ping_addr = tour->source_ip;
			ping_active = 1;
			ping_confirmed = 0;
			
			join_multicast(tour->multicast_ip, tour->multicast_port);
		}
		
		visited = 1;
		tour->current_tour++;
		if (tour->tour_ip[tour->current_tour] != 0)
		{
			send_tourpacket(tour, tour->tour_ip[tour->current_tour]);
		}
		else {
			ping_lastnode = 1;
		}
	}
}

// Print out message when a ICMP echo reply is received.
// Also start multicast message if 5th ping reply is received
void handle_echoresponse()
{
	char buf[5000];
	int bytes_received;
	
	bytes_received = recv(sock_icmp_resp, buf + sizeof(ethHdr), 5000, 0);
	if (bytes_received == sizeof(struct icmpheader) + sizeof(struct ip)) {
		echopkt *pkt = (echopkt *)&buf;
		
		if (pkt->icmph.icmp_type == 0) {
			if (pkt->iph.ip_src.s_addr == ping_addr) {
				printf("    PING response. %d bytes from vm%d: ttl=%d\n", bytes_received, get_vm_index(pkt->iph.ip_src.s_addr) + 1, pkt->iph.ip_ttl);
				ping_confirmed++;
				
				if (ping_confirmed == 5 && ping_lastnode == 1) {
				
					sprintf(buf, "<<<<< This is node vm%d. Tour has ended. Group members please identify yourselves. >>>>>", vm_me + 1);
					send_multicast(buf);
					ping_active = 0;
				}
			}
			else {
				printf("    PING response. %d bytes from someone other than source\n", bytes_received);
			}
		}
	}
}


int main(int argc, char* argv[]) 
{
	if (argc > 1) {
		printf("I am the source VM.\n");
		is_source = 1;
	}
	else {
		printf("I am NOT the source.\n");
		is_source = 0;
	}
	
	visited = 0;
	ping_active = 0;
	ping_lastnode = 0;
	shutdown_time = 0;
	
	init_vm_recognition();
	
	if (vm_me == -1) {
		printf("Not running on VM's. Exiting...\n");
		exit(1);
	}
	
	// Setup sock_tour
	{
		int one = 1;
		const int *val = &one;
		sock_tour = socket (PF_INET, SOCK_RAW, TOUR_PROTOCOL_ID);
		if (sock_tour < 0) {
			printf("Error creating socket\n");
			exit(1);
		}
		
		if (setsockopt (sock_tour, IPPROTO_IP, IP_HDRINCL, val, sizeof (one)) < 0) {
			printf ("Cannot set HDRINCL!\n");
			exit(1);
		}
		
		struct sockaddr_in sin;
		memset(&sin, 0, sizeof(sin));
		sin.sin_family = PF_INET;
		
		int err = bind(sock_tour, (struct sockaddr *)&sin, sizeof(sin));
		if (err < 0) {
			printf("error in bind\n");
			exit(1);
		}
	}
	
	// Setup sock_icmp_req
	{
		sock_icmp_req = socket (PF_PACKET, SOCK_RAW, IPPROTO_ICMP);
		if (sock_icmp_req < 0)
		{
			printf("Error creating icmp req socket\n");
			exit(1);
		}
	}
	
	// Setup sock_icmp_resp
	{
		sock_icmp_resp = socket (PF_INET, SOCK_RAW, IPPROTO_ICMP);
		if (sock_icmp_resp < 0)
		{
			printf("Error creating socket icmp response\n");
			exit(1);
		}
	}
	
	// Setup the multicast group socket, but don't join yet
	{
		sock_multicast = socket(AF_INET, SOCK_DGRAM, 0);
		if (sock_multicast < 0) {
			printf("Error creating socket for multicast");
			exit(1);
		}
		
		int one = 1;
		const int *val = &one;
		if (setsockopt(sock_multicast, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(one)) < 0) 
		{
			perror("Reusing ADDR failed");
			exit(1);
		}
	}
	
	printf("I am vm#%d\n", vm_me + 1);
	if (is_source) {
		build_tour(argc - 1, argv + 1);
		join_multicast(inet_addr(MULTICAST_IP), MULTICAST_PORT);
	}
	 
	fd_set rset;
	struct timeval tv;
	int select_result;
	
	time(&current_time);
	time_t next_ping = current_time;
	while (1)
	{
		
		tv.tv_sec = 0;
		tv.tv_usec = 200;

		FD_ZERO(&rset);
		FD_SET(sock_tour, &rset);
		FD_SET(sock_icmp_resp, &rset);
		FD_SET(sock_multicast, &rset);
		
		
		select_result = select(sock_multicast + 1, &rset, NULL, NULL, &tv);
		time(&current_time);
		
		if (FD_ISSET(sock_tour, &rset))
		{ 
			handle_tourpacket();
		}
		
		if (FD_ISSET(sock_icmp_resp, &rset))
		{ 
			handle_echoresponse();
		}
		
		if (FD_ISSET(sock_multicast, &rset))
		{
			handle_multicast();
		}
		
		if (next_ping <= current_time && ping_active == 1)
		{
			printf("PING vm%d:\n", + get_vm_index(ping_addr) + 1);
			send_icmp_echo_request(ping_addr);
			next_ping = current_time + 1;
		}
		
		if (shutdown_time != 0 && shutdown_time < current_time)
		{
			printf("Tour application is complete, exiting\n");
			exit(1);
		}
	}
	
	return 0;
}
