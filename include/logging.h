#ifndef _NET_LOGGING_H_
#define _NET_LOGGING_H_

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

// prints to stderr
void log_info(const char *msg);

// prints to stderr and aborts
void log_fatal(const char *msg);

#endif // _NET_LOGGING_H_
