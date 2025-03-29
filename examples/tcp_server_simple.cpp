#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <array>
#include <cassert>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <string_view>

#include "buffer.hpp"
#include "logging.hpp"

static constexpr std::size_t kMaxHeaderSize = 4;
static constexpr std::size_t kMaxMessageSize = 4096;
static constexpr std::string_view reply = "world";

static constexpr std::size_t kReadBufferSize = kMaxMessageSize + kMaxMessageSize;

auto Respond(int connfd) -> int32_t
{
    std::array<char, kReadBufferSize> read_buffer;
    errno = 0; // reset
    int32_t err = Net::Buffer::ReadN(connfd, read_buffer.data(), kMaxHeaderSize);
    if (err) {
        Net::Logging::LogInfo(errno == 0 ? "EOF"
                                         : "error in reading length of message");
        return err;
    }
    uint32_t length = 0;
    memcpy(&length, read_buffer.data(), kMaxHeaderSize); // assumes little endian
    if (length > kMaxMessageSize) {
        Net::Logging::LogInfo("length of message too long");
        return -1;
    }
    err = Net::Buffer::ReadN(connfd, &read_buffer[kMaxHeaderSize], length);
    if (err) {
        Net::Logging::LogInfo("error in reading message");
        return err;
    }
    std::string_view got { &read_buffer[kMaxHeaderSize], length };
    std::cout << "client says: " << got << '\n';

    std::string write_buffer;
    write_buffer.reserve(kMaxHeaderSize + reply.size());
    length = reply.size();
    memcpy(write_buffer.data(), &length, kMaxHeaderSize);
    memcpy(&write_buffer[kMaxHeaderSize], reply.data(), length);
    return Net::Buffer::WriteN(connfd, write_buffer.c_str(),
        kMaxHeaderSize + length);
}

auto main() -> int
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        Net::Logging::LogFatal("failed to construct socket");
    }

    int optval = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = ntohl(0);
    addr.sin_port = ntohs(1234);
    int rv = bind(fd, reinterpret_cast<const sockaddr*>(&addr), sizeof(addr));
    if (rv) {
        Net::Logging::LogFatal("failed to bind");
    }

    rv = listen(fd, SOMAXCONN);
    if (rv) {
        Net::Logging::LogFatal("failed to listen");
    }

    while (1) {
        sockaddr_in client_addr = {};
        socklen_t socklength = sizeof(client_addr);
        int connfd = accept(fd, reinterpret_cast<sockaddr*>(&client_addr), &socklength);
        if (connfd < 0) {
            continue;
        }

        while (1) {
            if (int32_t err = Respond(connfd); err) {
                break;
            }
        }
        close(connfd);
    }

    return 0;
}
