#include <arpa/inet.h>
#include <sys/socket.h>

#include <array>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string_view>

#include "buffer.hpp"
#include "logging.hpp"

static constexpr std::size_t kMaxHeaderSize = 4;
static constexpr std::size_t kMaxMessageSize = 4096;

static constexpr std::size_t kWriteBufferSize = kMaxHeaderSize + kMaxMessageSize;
static constexpr std::size_t kReadBufferSize = kMaxHeaderSize + kMaxMessageSize + 1;

auto Query(int fd, std::string_view message) -> int
{
    uint32_t length = message.length();
    if (length > kMaxMessageSize) {
        return -1;
    }

    std::array<char, kWriteBufferSize> write_buffer;
    memcpy(write_buffer.data(), &length, kMaxHeaderSize);
    memcpy(&write_buffer[kMaxHeaderSize], message.data(), length);
    if (int32_t err = Net::Buffer::WriteN(fd, write_buffer.data(),
            kMaxHeaderSize + length)) {
        return err;
    }

    std::array<char, kReadBufferSize> read_buffer;
    errno = 0;
    if (int32_t err = Net::Buffer::ReadN(fd, read_buffer.data(), kMaxHeaderSize)) {
        Net::Logging::LogInfo(errno == 0 ? "EOF" : "error in reading length");
        return err;
    }
    memcpy(&length, read_buffer.data(), kMaxHeaderSize);
    if (length > kMaxMessageSize) {
        Net::Logging::LogInfo("message too long");
        return -1;
    }

    if (int32_t err = Net::Buffer::ReadN(fd, &read_buffer[kMaxHeaderSize], length)) {
        Net::Logging::LogInfo("error in reading message");
        return err;
    }

    std::string_view got { &read_buffer[kMaxHeaderSize], length };
    std::cout << "server says: " << got << '\n';
    return 0;
}

auto main() -> int
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        Net::Logging::LogFatal("failed to construct socket");
    }

    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = ntohl(INADDR_LOOPBACK);
    addr.sin_port = ntohs(1234);
    if (int dial = connect(fd, reinterpret_cast<const struct sockaddr*>(&addr),
            sizeof(addr));
        dial) {
        Net::Logging::LogFatal("failed to connect");
    }

    if (Query(fd, "hello1")) {
        Net::Logging::LogFatal("failed to send hello1");
    }
    if (Query(fd, "hello2")) {
        Net::Logging::LogFatal("failed to send hello2");
    }

    close(fd);
    return 0;
}
