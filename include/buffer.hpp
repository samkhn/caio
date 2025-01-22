#ifndef _NET_BUFFER_H_
#define _NET_BUFFER_H_

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

// reads n bytes into buffer from fd
int32_t read_full(int fd, char *buf, size_t n);

// writes n bytes from buf into fd
int32_t write_all(int fd, const char *buf, size_t n);

#endif  // _NET_BUFFER_H_
