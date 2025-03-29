#ifndef _CAIO_LOGGING_H_
#define _CAIO_LOGGING_H_

#include <stdlib.h>

namespace Caio::Logging {

// prints to stderr
void LogInfo(const char* msg);

// prints to stderr and aborts
void LogFatal(const char* msg);

} // namespace Caio::Logging

#endif // _CAIO_LOGGING_H_
