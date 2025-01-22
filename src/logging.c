#include "logging.h"

void log_info(const char *msg)
{
	fprintf(stderr, "%s\n", msg);
}

void log_fatal(const char *msg)
{
	int err = errno;
	fprintf(stderr, "[%d] %s\n", err, msg);
	abort();
}
