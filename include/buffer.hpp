#ifndef _NET_BUFFER_H_
#define _NET_BUFFER_H_

#include <assert.h>
#include <stdlib.h>
#include <unistd.h>

namespace Net {
namespace Buffer {

// reads n bytes into buffer from fd
int32_t ReadN(int fd, char* buf, size_t n);

// writes n bytes from buf into fd
int32_t WriteN(int fd, const char* buf, size_t n);

}  // namespace Buffer
}  // namespace Net

#endif  // _NET_BUFFER_H_
