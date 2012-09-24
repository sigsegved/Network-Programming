Group 8:
Richard Meester
Shah Pavel Jamal
Karthik Uthaman


The modifications we made to unpifiplus.h and get_ifi_info_plus.c in order to accomidate easy calculation of of the subnet address was the following: 

In struct ifi_info we added an additional pointer to a sockaddr which will store the network id.

In get_ifi_info_plus.c we added a section in the code which bitwise ands the ipaddress and the subnet mask to calculate the network ID. 

We also needed to modify the free_ifi_info_plus function to ensure that the memory allocated for the network id is properly freed.

The code which runs the server is detailed in server.c. It operates in the following fashion:

The server begins by setting up the signal handler to reap child processes which have completed execution. It then reads server.in to get the initial operating settings for the server (port number and window size)

Next it gets the address information for each interface and binds appropriately to non-broadcast addresses. At this point we also set up the file descriptor set for I/O.

After that initialization we enter the listening loop. The server waits for a UDP packet on any socket, at which point, upon reception the server examines the address information for the client to determine locality. If its local, we turn off routing. At this point we create a new socket which is to be used to transfer the file with the client. we then transfer the port number to the client and fork a child process. The parent process then waits for more connetions.

The child process then does the following: 

It attempts to open the filename which was provided by the client. If this fails it sends the client -1 and exits. Otherwise it calculates the file length and transmits that to the client, and begins the file transfer.

The code which runs the client is detailed in updcli.c. 
Initially we loaded all the file settings from client.in for future use. We then checked for locality by using get_ifi_info_plus that was given to us to get all the network interfaces. Afterwards we started listening for server communication on 1 port and start initial communication with the server by sending it a NULL terminated filename. Once the server returned Port number for the file transfer, the client sends an OK to the server from the new port and then server begins streaming to the client. Initially the server sends the filesize, and then sends the actual content. Once the whole file is recieved the program successfully shuts down.

RELIABLITY : 

Both the server and client maintains their own sliding window of different sizes.  For the sake of variation the server's sliding window has been implemented as a circular array, where as the clients is just a simple array. The main component of the reliablity part is the sliding window. The following structure has been used for maintaining the window status. 

typedef struct _window
{
        tcpPacket windElems[MAX_WNDW_SIZE];	 	//--> Stores the data with the header.
        int wndw_size;							//--> Windows Size
        int cwndw_size;							//--> Current free window space
        int LFA;								//--> LAST FRAME ACKNOWLEDGED 
        int LFR;								//--> LAST FRAME REQUESTED
        int CRP;								//--> CURRENT READ POINTER
        int CWP;								//--> CURRENT WRITE POINTER
        int state;								//--> WINDOW STATE
}window;


To add flow control and congestion control, we use a header to carry information about each packet and the current state of the client sliding window. The header looks as shown below
typedef struct _header
{
        sequence seqNo;							//64 Bit Sequence Number
        sequence ackNo;							//64 bit ACK
        int type;								//TYPE OF THE PACKET -> ACK,FIN,FIN_ACK,PSH,SYN...
        int msglen;								//PAYLOAD LENGTH
        int cwndw_size;							//CURRENT WINDOW SIZE  --> Congestion control
        float drop_probablity;					//DRop probablity...
}tcpHeader;

Each TCP PACKET looks as shown below. 
typedef struct _packet
{
        tcpHeader header;						//TCP HEADER
        char data[MAX_DATA_LEN];				//TCP PAYLOAD
}tcpPacket;

--------------
ACK SYSTEM... 
--------------
We follow a cummulative ACK system, where a ACK is sent for each packet and the acknowledgement packet always contains the last packet received in order. 

------------------
Re-Transmission ... 
------------------
After receiving the ACKs, the server will get to know the current window size (m) of the client and the ack value. Now the server will send m packets back to client starting from the last ack received. 

--------------------
Congestion Control... 
--------------------
Since every ack carries the advertised window size, server will not overload the client anytime and so the congestion is avoided.

-------------------
Timeout mechanism... 
-------------------
RTT is initially calculated and stored in the rrt variables as mentioned in the stevens code. For every ACK received we recalculate the RTT and use it in the select's timeout variable... 
This way we can get rid of the sigalarm and the sigsetjmp functions. If any ACK is lost it will not be received in 2xRTT time, as our select will be waiting for 2xRTT time all the time, and if we do not get an ACK then we can say that the ACK is lost. 
When an ACK is lost, we re-transmit the packets from the lost ack number to no.of packet the client can accomodate right now. As the client maintains the LFR variable, it will simply ignore if the seq of the packet is not equal to LFR+1 and send ACK with seq.no LFR+1. 

--------------
DUP ACKS : 
--------------
Duplicate acks are avoided by letting the client's print thread to consume the window data, when the window is full. So we will not get a duplicate ACK at anytime. 



FILES : 
recv.c --> Contains the code for receivers sliding window protocol.
rsend.c --> Contains the code for senders sliding window protocol. 

SERVER FLOW : 
1. Sender greedily sends m packets, where m is the advertised client window size. 
2. When sender can no more send data or the senders sliding window is full, it reads the ACK from client, to get the current window size.
3. On reading the acks, sender do the following things,
   a. Cleans up the window for correctly transmitted packets
   b. Retransmits the lost packets
   c. updates the latest client window size
4. Initiates the active close 
5. Gets back accepting more connections.

CLIENT FLOW : 
1. Reads packets one by one and send an ack back. 
2. If the packet read is not accpetable, it drops the packets and sends an ACK for the last packet received in order.
3. When the window is full, the print thread is asked to consume the data and print it on the console. 
4. When all the packets are received, the connection is closed passively.



