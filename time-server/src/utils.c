/*
 ============================================================================
 Name        : utils.c
 Author      : Karthik Uthaman
 Version     :
 Copyright   : Published under GNU License.
 Description :
 Created on	 : Sep 22, 2011
 ============================================================================
 /*
 * Socket wrapper functions.
 * These could all go into separate files, so only the ones needed cause
 * the corresponding function to be added to the executable.  If sockets
 * are a library (SVR4) this might make a difference (?), but if sockets
 * are in the kernel (BSD) it doesn't matter.
 *
 * These wrapper functions also use the same prototypes as POSIX.1g,
 * which might differ from many implementations (i.e., POSIX.1g specifies
 * the fourth argument to getsockopt() as "void *", not "char *").
 *
 * If your system's headers are not correct [i.e., the Solaris 2.5
 * <sys/socket.h> omits the "const" from the second argument to both
 * bind() and connect()], you'll get warnings of the form:
 *warning: passing arg 2 of `bind' discards `const' from pointer target type
 *warning: passing arg 2 of `connect' discards `const' from pointer target type
 */


#include "utils.h"

static int	read_cnt;
static char	*read_ptr;
static char	read_buf[MAXLINE];

int _Readline(int sockd, void *vptr, int maxlen) ;
int Getsockaddr(char *hostaddr, struct sockaddr_in *serv_addr);

/*Getsockaddr is used to resolve the ip/hostname issue for the client process. It uses the getaddrinfo. */

int Getsockaddr(char *hostaddr, struct sockaddr_in *serv_addr)
{
    struct addrinfo hints, *res, *peer;
    int status;
    char ipstr[INET6_ADDRSTRLEN];
	int rc=0;

    memset(&hints, 0, sizeof hints);
	//To hint the getaddrinfo about the host...
    hints.ai_family = AF_INET; 			// AF_INET or AF_INET6 to force version
    hints.ai_socktype = SOCK_STREAM;		//TCP 

    if ((status = getaddrinfo(hostaddr, NULL, &hints, &res)) != 0) 
    {
        fprintf(stderr, "getaddrinfo: %s\n", strerror(errno));
	rc = -1;
    }
    else
    {



    for(peer = res;peer != NULL; peer = peer->ai_next) 	
    {
        void *addr;

        // get the pointer to the address itself,
	// Check to see if the IP address is IPV4... 
        if (peer->ai_family == AF_INET) 
	{ 
            struct sockaddr_in *ip = (struct sockaddr_in *)peer->ai_addr;
            addr = &(ip->sin_addr);
	    memcpy(serv_addr,ip,sizeof(struct sockaddr_in));
   	    inet_ntop(peer->ai_family, &(serv_addr->sin_addr), ipstr, sizeof ipstr);
		//go for the first available IP address...
	    break;
        }
    }
    freeaddrinfo(res); // free the linked list
   }
	return rc;

}


/* FUNCTION TO RESOLVE IP ADDRESS AND HOSTNAME AND GET THE HOSTENT STRUCTURE THAT CAN BE USED FURTHER*/
int Gethostent(char *address, struct hostent *hptr)
{

	char *p1;
	int errcode = FAILURE;
	struct hostent *hostptr;
	struct in_addr addrptr;
	if(address)
	{
		//errcode = inet_aton(address,&addrptr);
		if(errcode == 0)
		{
			if((hostptr=gethostbyaddr((char *)&addrptr,SOCK_LEN_IPV4,AF_INET))==NULL)
				errcode = errno;
			else
				errcode = SUCCESS;
		}
		else
		{
			if((hostptr=gethostbyname(address))==NULL)
				errcode = errno;
			else
				errcode = SUCCESS;
		}
	}
	if(hostptr)
		memcpy(hptr,hostptr,sizeof(struct hostent));


	return errcode;
}

void err_msg(int errcode, char *errmsg)
{
	printf("%d : %s : %s",errcode,errmsg,strerror(errcode));
}

/* A simple wrapper for connect to handle error conditions*/

int tcpConnect(int serv_sockfd, struct sockaddr_in serv_addr,int serv_port)
{
	int isConnected = false;
        char ipstr[20];
        memset(ipstr,0,sizeof(ipstr));
        inet_ntop(AF_INET, &(serv_addr.sin_addr), ipstr, sizeof(ipstr));

	if(Connect(serv_sockfd,(struct sockaddr *)&serv_addr,sizeof(struct sockaddr))==NO_ERROR)
	{
		printf("Connected to server\n IP : %s \n Port : %d\n\n",inet_ntoa(serv_addr.sin_addr),serv_port);
		isConnected = true;
	}
	return isConnected;
}
/*
int createtcpsocket(int serv_port, struct sockaddr_in *s_addr)
{
	int serv_sockfd;
	s_addr->sin_port = serv_port;
	serv_sockfd = Socket(AF_INET, SOCK_STREAM, 0);
	return serv_sockfd;

}
*/
int tcpListen(int service_sockfd,struct sockaddr_in sa)
{
	//BIND
	//LISTEN and RETURN ERROR CODE
	int bl;

	Bind(service_sockfd,(struct sockaddr *)&sa,sizeof(struct sockaddr));
	Listen(service_sockfd,bl);
}

int
Accept(int fd, struct sockaddr *sa, socklen_t *salenptr)
{
	int		n;

again:
	if ( (n = accept(fd, sa, salenptr)) < 0) {
#ifdef	EPROTO
		if (errno == EPROTO || errno == ECONNABORTED)
#else
		if (errno == ECONNABORTED)
#endif
			goto again;
		else
			err_msg(errno,"accept error");
	}
	return(n);
}

void
Bind(int fd, const struct sockaddr *sa, socklen_t salen)
{
	if (bind(fd, sa, salen) < 0)
		err_msg(errno,"bind error");
}

int
Connect(int fd, const struct sockaddr *sa, socklen_t salen)
{
	int rc = NO_ERROR;
	if ((rc=connect(fd, sa, salen)) < 0)
		err_msg(errno,"connect error");

	return rc;
}

void
Getpeername(int fd, struct sockaddr *sa, socklen_t *salenptr)
{
	if (getpeername(fd, sa, salenptr) < 0)
		err_msg(errno,"getpeername error");
}

void
Getsockname(int fd, struct sockaddr *sa, socklen_t *salenptr)
{
	if (getsockname(fd, sa, salenptr) < 0)
		err_msg(errno,"getsockname error");
}

void
Getsockopt(int fd, int level, int optname, void *optval, socklen_t *optlenptr)
{
	if (getsockopt(fd, level, optname, optval, optlenptr) < 0)
		err_msg(errno,"getsockopt error");
}



/* include Listen */
void
Listen(int fd, int backlog)
{
	char	*ptr;

		/*4can override 2nd argument with environment variable */
	if ( (ptr = getenv("LISTENQ")) != NULL)
		backlog = atoi(ptr);

	if (listen(fd, backlog) < 0)
		err_msg(errno,"listen error");
}
/* end Listen */

ssize_t
Recv(int fd, void *ptr, size_t nbytes, int flags)
{
	ssize_t		n;

	if ( (n = recv(fd, ptr, nbytes, flags)) < 0)
		err_msg(errno,"recv error");
	return(n);
}

ssize_t
Recvfrom(int fd, void *ptr, size_t nbytes, int flags,
		 struct sockaddr *sa, socklen_t *salenptr)
{
	ssize_t		n;

	if ( (n = recvfrom(fd, ptr, nbytes, flags, sa, salenptr)) < 0)
		err_msg(errno,"recvfrom error");
	return(n);
}

ssize_t
Recvmsg(int fd, struct msghdr *msg, int flags)
{
	ssize_t		n;

	if ( (n = recvmsg(fd, msg, flags)) < 0)
		err_msg(errno,"recvmsg error");
	return(n);
}

int
Select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
       struct timeval *timeout)
{
	int		n;

	if ( (n = select(nfds, readfds, writefds, exceptfds, timeout)) < 0)
		err_msg(errno,"select error");
	return(n);		/* can return 0 on timeout */
}

void
Send(int fd, const void *ptr, size_t nbytes, int flags)
{
	if (send(fd, ptr, nbytes, flags) != (ssize_t)nbytes)
		err_msg(errno,"send error");
}

void
Sendto(int fd, const void *ptr, size_t nbytes, int flags,
	   const struct sockaddr *sa, socklen_t salen)
{
	if (sendto(fd, ptr, nbytes, flags, sa, salen) != (ssize_t)nbytes)
		err_msg(errno,"sendto error");
}

void
Sendmsg(int fd, const struct msghdr *msg, int flags)
{
	unsigned int	i;
	ssize_t			nbytes;

	nbytes = 0;	/* must first figure out what return value should be */
	for (i = 0; i < msg->msg_iovlen; i++)
		nbytes += msg->msg_iov[i].iov_len;

	if (sendmsg(fd, msg, flags) != nbytes)
		err_msg(errno,"sendmsg error");
}

void
Setsockopt(int fd, int level, int optname, const void *optval, socklen_t optlen)
{
	if (setsockopt(fd, level, optname, optval, optlen) < 0)
		err_msg(errno,"setsockopt error");
}

void
Shutdown(int fd, int how)
{
	if (shutdown(fd, how) < 0)
		err_msg(errno,"shutdown error");
}

int
Sockatmark(int fd)
{
	int		n;

	if ( (n = sockatmark(fd)) < 0)
		err_msg(errno,"sockatmark error");
	return(n);
}

/* include Socket */
int
Socket(int family, int type, int protocol)
{
	int		n;

	if ( (n = socket(family, type, protocol)) < 0)
		err_msg(errno,"socket error");
	return(n);
}
/* end Socket */

void
Socketpair(int family, int type, int protocol, int *fd)
{
	int		n;

	if ( (n = socketpair(family, type, protocol, fd)) < 0)
		("socketpair error");
}


ssize_t						/* Write "n" bytes to a descriptor. */
writen(int fd, const void *vptr, size_t n)
{
	size_t		nleft;
	ssize_t		nwritten;
	const char	*ptr;

	ptr = vptr;
	nleft = n;
	while (nleft > 0) {
		if ( (nwritten = write(fd, ptr, nleft)) <= 0) {
			if (nwritten < 0 && errno == EINTR)
				nwritten = 0;		/* and call write() again */
			else
				return(-1);			/* error */
		}

		nleft -= nwritten;
		ptr   += nwritten;
	}
	return(n);
}

void
Writen(int fd, void *ptr, size_t nbytes)
{
	if (writen(fd, ptr, nbytes) != nbytes)
		err_msg(errno,"writen error");
}


ssize_t						/* Read "n" bytes from a descriptor. */
readn(int fd, void *vptr, size_t n)
{
	size_t	nleft;
	ssize_t	nread;
	char	*ptr;

	ptr = vptr;
	nleft = n;
	while (nleft > 0) {
		if ( (nread = read(fd, ptr, nleft)) < 0) {
			if (errno == EINTR)
				nread = 0;		/* and call read() again */
			else
				return(-1);
		} else if (nread == 0)
			break;				/* EOF */

		nleft -= nread;
		ptr   += nread;
	}
	return(n - nleft);		/* return >= 0 */
}
/* end readn */


ssize_t
Readn(int fd, void *ptr, size_t nbytes)
{
	ssize_t		n;

	if ( (n = readn(fd, ptr, nbytes)) < 0)
		err_msg(errno,"readn error");
	return(n);
}


int write_pipe(int pfd,char *buff,int size)
{
	int rc = writen(pfd,buff,size);
	if(rc == -1)
	{
		printf("PIPE CLOSED. I could be an orphan process now.. :(\n");
		close(pfd);
		//close_client();
	}

	return rc;
}


int _Readline(int sockd, void *vptr, int maxlen) {
    int n, rc;
    char    c, *buffer;

    buffer = vptr;

    for ( n = 1; n < maxlen; n++ ) {
	
	if ( (rc = read(sockd, &c, 1)) == 1 ) {
	    *buffer++ = c;
	    if ( c == '\n' )
	    {
		break;
		}
	}
	else if ( rc == 0 ) {
	    if ( n == 1 )
		return 0;
	    else
		break;
	}
	else {
	    if ( errno == EINTR )
		continue;
	    return -1;
	}
    }

    *buffer = 0;
    return n;
}



static ssize_t
my_read(int fd, char *ptr)
{

	if (read_cnt <= 0) {
again:
		if ( (read_cnt = read(fd, read_buf, sizeof(read_buf))) < 0) {
			if (errno == EINTR)
				goto again;
			return(-1);
		} else if (read_cnt == 0)
			return(0);
		read_ptr = read_buf;
	}

	read_cnt--;
	*ptr = *read_ptr++;
	return(1);
}

ssize_t
readline(int fd, void *vptr, size_t maxlen)
{
	ssize_t	n, rc;
	char	c, *ptr;

	ptr = vptr;
	for (n = 1; n < maxlen; n++) {
		if ( (rc = my_read(fd, &c)) == 1) {
			*ptr++ = c;
			if (c == '\n')
				break;	/* newline is stored, like fgets() */
		} else if (rc == 0) {
			*ptr = 0;
			return(n - 1);	/* EOF, n - 1 bytes were read */
		} else
			return(-1);		/* error, errno set by read() */
	}

	*ptr = 0;	/* null terminate like fgets() */
	return(n);
}

ssize_t
readlinebuf(void **vptrptr)
{
	if (read_cnt)
		*vptrptr = read_ptr;
	return(read_cnt);
}
/* end readline */

ssize_t
Readline(int fd, void *ptr, size_t maxlen)
{
	ssize_t		n;

	if ( (n = readline(fd, ptr, maxlen)) < 0)
		err_msg(errno,strerror(errno));
	return(n);
}


