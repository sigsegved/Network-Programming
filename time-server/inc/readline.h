#ifndef __READLINE_H
#define __READLIEN_H


#include "cs533.h"

static ssize_t my_read(int fd, char *ptr);
ssize_t readline(int fd, void *vptr, size_t maxlen);
ssize_t readlinebuf(void **vptrptr);
ssize_t Readline(int fd, void *ptr, size_t maxlen);

#endif
