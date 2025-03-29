#include <cerrno>

#include "buffer.hpp"

namespace Caio::Buffer {

auto ReadN(int fd, char* buf, size_t n) -> int32_t
{
    while (n > 0) {
        ssize_t rv = read(fd, buf, n);
        if (rv <= 0) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        assert((size_t)rv <= n);
        n -= static_cast<size_t>(rv);
        buf += rv;
    }
    return 0;
}

auto WriteN(int fd, const char* buf, size_t n) -> int32_t
{
    while (n > 0) {
        ssize_t rv = write(fd, buf, n);
        if (rv <= 0) {
            return -1;
        }
        assert(static_cast<size_t>(rv) <= n);
        n -= static_cast<size_t>(rv);
        buf += rv;
    }
    return 0;
}

} // namespace Caio::Buffer
