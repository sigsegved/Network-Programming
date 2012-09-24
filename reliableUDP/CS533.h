#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <sys/fcntl.h>
#include "unpifiplus.h"

#define	max(a,b)	((a) > (b) ? (a) : (b))
#define FRAMEBUFFERSIZE 508
struct frame
{
  int sequence_id; //4 Bytes
  char buffer[FRAMEBUFFERSIZE];  //Change the define if you add other fields
}; //sizeof(frame) should be 512

struct ack_frame
{
  int sequence_id;
};

struct init_frame
{
  int sequence_id; //for init frame, should be 0
  int total_frame_count; //total number of frames to be sent
  int total_byte_count; //sizeof(entire message)
};
/*
int Accept(int fd, struct sockaddr *sa, socklen_t *salenptr)
{
  int n;
  
 again:
  if((n = accept(fd, sa, salenptr)) < 0)
    {
      
      if ( errno == EPROTO 		|| errno == ECONNABORTED 	|| errno == ENETDOWN ||
	   errno == ENOPROTOOPT 	|| errno == EHOSTDOWN 		|| errno == ENONET ||
	   errno == EHOSTUNREACH 	|| errno == EOPNOTSUPP 		|| errno == ENETUNREACH)
	goto again;
      else
	fprintf(stderr, "Accept Error\n");
      
    }
  return n;
}

int Connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
  int n;
 again:
  
  if((n = connect(sockfd, addr, addrlen))< 0)
    {
      switch(errno)
	{
	case EACCES:
	  perror("EACCESS");
	  exit(0);
	  break;
	case EPERM:
	  perror("EPERM");
	  exit(0);
	  break;
	case EADDRINUSE:
	  perror("EADDRINUSE");
	  exit(0);
	  break;
	case EAFNOSUPPORT:
	  perror("EAFNOSUPPORT");
	  exit(0);
	  break;
	case EAGAIN:
	  perror("EAGAIN");
	  goto again;
	  break;
	case EALREADY:
	  perror("EALREADY");
	  exit(0);
	  break;
	case EBADF:
	  perror("EBADF");
	  exit(0);
	  break;
	case ECONNREFUSED:
	  perror("ECONNREFUSED");
	  exit(0);
	  break;
	case EFAULT:
	  perror("EFAULT");
	  exit(0);
	  break;
	case EINPROGRESS:
	  perror("EINPROGRESS");
	  exit(0);
	  break;
	case EINTR:
	  perror("EINTR");
	  goto again;
	  break;
	case EISCONN:
	  perror("EISCONN");
	  break;
	case ENETUNREACH:
	  perror("ENETUNREACH");
	  exit(0);
	  break;
	case ENOTSOCK:
	  perror("ENOTSOCK");
	  exit(0);
	  break;
	case ETIMEDOUT:
	  perror("ETIMEDOUT");
	  exit(0);
	  break;
	default:
	  perror("OTHER");
	  exit(0);
	  break;
	}
    }
  return n;
}

ssize_t Readn(int fd, const void *buff, size_t len)
{
  size_t bytesleft;
  ssize_t bytesread;
  char *buffer;
  
  buffer = (char *)buff;
  bytesleft = len;
  
  while(bytesleft > 0)
    {
      if ( (bytesread = read(fd, buffer, bytesleft)) < 0)
	{
	  if(errno == EINTR)
	    bytesread = 0;
	  else
	    return -1;
	}
      else if (bytesread == 0)
	break;
      
      bytesleft -= bytesread;
      buffer += bytesread;
    }
  return (len - bytesleft);
}

ssize_t Writen(int fd, const void *vptr, size_t n)
{
  size_t bytesleft;
  ssize_t byteswritten;
  const char *ptr;
  
  ptr = vptr;
  bytesleft = n;
  
  while(bytesleft > 0)
    {
      if((byteswritten = write(fd, ptr, bytesleft)) <= 0)
	{
	  if(byteswritten <= 0 && errno == EINTR)
	    {
	      byteswritten = 0;
	    }
	  else
	    {
	      return -1;
	    }
	}
      bytesleft -= byteswritten;
      ptr += byteswritten;
    }
  return n;
}
*/
