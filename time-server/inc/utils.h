/*
 * utils.h
 *
 *  Created on: Sep 22, 2011
 *      Author: ukay
 */

#ifndef UTILS_H_
#define UTILS_H_

#include "cs533.h"

int Gethostent(char *address, struct hostent *hptr);
void err_msg(int errcode, char *errmsg);
int tcpConnect(int serv_sockfd, struct sockaddr_in serv_addr,int serv_port);
//int createtcpsocket(int serv_port, struct sockaddr_in *s_addr);
int tcpListen(int service_sockfd, struct sockaddr_in );
int Accept(int fd, struct sockaddr *sa, socklen_t *salenptr);
void Bind(int fd, const struct sockaddr *sa, socklen_t salen);
int Connect(int fd, const struct sockaddr *sa, socklen_t salen);
void Getpeername(int fd, struct sockaddr *sa, socklen_t *salenptr);
void Getsockopt(int fd, int level, int optname, void *optval, socklen_t *optlenptr);
void Listen(int fd, int backlog);
ssize_t Recv(int fd, void *ptr, size_t nbytes, int flags);
ssize_t Recvfrom(int fd, void *ptr, size_t nbytes, int flags, struct sockaddr *sa, socklen_t *salenptr);
ssize_t Recvmsg(int fd, struct msghdr *msg, int flags);
int Select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);
void Send(int fd, const void *ptr, size_t nbytes, int flags);
void Sendto(int fd, const void *ptr, size_t nbytes, int flags, const struct sockaddr *sa, socklen_t salen);
void Sendmsg(int fd, const struct msghdr *msg, int flags);
ssize_t
Recvmsg(int fd, struct msghdr *msg, int flags);

int Select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
       struct timeval *timeout);

void Send(int fd, const void *ptr, size_t nbytes, int flags);

void Sendto(int fd, const void *ptr, size_t nbytes, int flags,
           const struct sockaddr *sa, socklen_t salen);
void Sendmsg(int fd, const struct msghdr *msg, int flags);

ssize_t                                         /* Write "n" bytes to a descriptor. */
writen(int fd, const void *vptr, size_t n);

void
Writen(int fd, void *ptr, size_t nbytes);



int write_pipe(int pfd,char *buff,int size);

ssize_t
Readn(int fd, void *ptr, size_t nbytes);


int write_pipe(int pfd,char *buff,int size);

static ssize_t my_read(int fd, char *ptr);
ssize_t readline(int fd, void *vptr, size_t maxlen);
ssize_t readlinebuf(void **vptrptr);
ssize_t Readline(int fd, void *ptr, size_t maxlen);



#endif /* UTILS_H_ */
