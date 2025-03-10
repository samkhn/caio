#ifndef _NET_LOGGING_H_
#define _NET_LOGGING_H_

#include <stdlib.h>

namespace Net {
namespace Logging {

// prints to stderr
void LogInfo(const char *msg);

// prints to stderr and aborts
void LogFatal(const char *msg);

}  // namespace Logging
}  // namespace Net

#endif  // _NET_LOGGING_H_
