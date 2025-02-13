#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include <memory>
#include <vector>

#include "logging.hpp"

static constexpr std::size_t kMaxHeaderSize = 4;
static constexpr std::size_t kMaxMessageSize = 4096;

enum class ConnectionStatus {
    OFF, // only during initialization
    READ,
    WRITE,
    CLOSE // sets up connection to be destroyed
};

struct Connection {
    int fd = -1;
    ConnectionStatus status = ConnectionStatus::OFF;
    std::vector<uint8_t> incoming;
    std::vector<uint8_t> outgoing;
};

// TODO: return error if we can't set it
void fd_set_nonblocking(int fd)
{
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);
}

// TODO: move next two methods into being class functions of Connection
void BufferAppend(std::vector<uint8_t>& buffer, const uint8_t* data,
    size_t length)
{
    buffer.insert(buffer.end(), data, data + length);
}

void BufferConsume(std::vector<uint8_t>& buffer, size_t n)
{
    buffer.erase(buffer.begin(), buffer.begin() + n);
}

std::unique_ptr<Connection> HandleAccept(int fd)
{
    sockaddr_in client_addr = {};
    socklen_t socklen = sizeof(client_addr);
    int connection_fd = accept(fd, reinterpret_cast<sockaddr*>(&client_addr), &socklen);
    if (connection_fd < 0) {
        return nullptr;
    }
    fd_set_nonblocking(connection_fd);
    std::unique_ptr<Connection> connection = std::make_unique<Connection>();
    uint32_t ip = client_addr.sin_addr.s_addr;
    fprintf(stderr, "new client from %u.%u.%u.%u:%u\n", ip & 255, (ip >> 8) & 255,
        (ip >> 16) & 255, ip >> 24, ntohs(client_addr.sin_port));
    connection->fd = connection_fd;
    // when we accept a request, we want to read it first
    connection->status = ConnectionStatus::READ;
    return connection;
}

std::unique_ptr<Connection> TryOneRequest(
    std::unique_ptr<Connection> connection)
{
    if (connection->incoming.size() < kMaxHeaderSize) {
        return connection; // we want to continue reading
    }
    uint32_t length = 0;
    memcpy(&length, connection->incoming.data(), kMaxHeaderSize);
    if (length > kMaxMessageSize) {
        // protocol error, message size exceeds limit
        connection->status = ConnectionStatus::CLOSE;
        return connection;
    }
    const uint8_t* request = &connection->incoming[4];
    BufferAppend(connection->outgoing, (const uint8_t*)&length, kMaxHeaderSize);
    BufferAppend(connection->outgoing, request, length);
    BufferConsume(connection->incoming, kMaxHeaderSize + length);
    return connection;
}

std::unique_ptr<Connection> HandleRead(std::unique_ptr<Connection> connection)
{
    uint8_t buffer[64 * 1024];
    ssize_t rv = read(connection->fd, buffer, sizeof(buffer));
    if (rv <= 0) {
        connection->status = ConnectionStatus::CLOSE;
        return connection;
    }
    BufferAppend(connection->incoming, buffer, (size_t)rv);
    std::unique_ptr<Connection> read_request = TryOneRequest(std::move(connection));
    if (read_request->outgoing.size() > 0) {
        // has a response
        read_request->status = ConnectionStatus::WRITE;
    } else {
        read_request->status = ConnectionStatus::READ;
    }
    return read_request;
}

std::unique_ptr<Connection> HandleWrite(
    std::unique_ptr<Connection> connection)
{
    assert(connection->outgoing.size() > 0);
    ssize_t rv = write(connection->fd, connection->outgoing.data(),
        connection->outgoing.size());
    if (rv < 0) {
        connection->status = ConnectionStatus::CLOSE;
        return connection;
    }
    BufferConsume(connection->outgoing, (size_t)rv);
    if (connection->outgoing.size() == 0) {
        connection->status = ConnectionStatus::READ;
    } else {
        connection->status = ConnectionStatus::WRITE;
    }
    return connection;
}

int main()
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

    std::vector<std::unique_ptr<Connection>> fd_to_connection;
    std::vector<pollfd> poll_args;
    while (1) {
        poll_args.clear();
        pollfd pfd_in = { fd, POLLIN, 0 };
        poll_args.push_back(pfd_in);
        for (auto& connection : fd_to_connection) {
            if (!connection) {
                continue;
            }
            pollfd pfd = { connection->fd, POLLERR, 0 };
            if (connection->status == ConnectionStatus::READ) {
                pfd.events |= POLLIN;
            }
            if (connection->status == ConnectionStatus::WRITE) {
                pfd.events |= POLLOUT;
            }
            poll_args.push_back(pfd);
        }
        int poll_result = poll(poll_args.data(), (nfds_t)poll_args.size(),
            -1); // blocking
        if (poll_result < 0 && errno == EINTR) {
            continue; // interrupted, not an error
        }
        if (poll_result < 0) {
            Net::Logging::LogFatal("poll");
        }
        if (poll_args[0].revents) {
            if (std::unique_ptr<Connection> connection = HandleAccept(fd)) {
                if (fd_to_connection.size() <= (size_t)connection->fd) {
                    fd_to_connection.resize(connection->fd + 1);
                }
                assert(!fd_to_connection[connection->fd]);
                fd_to_connection[connection->fd] = std::move(connection);
            }
        }
        for (size_t i = 1; i < poll_args.size(); i++) {
            uint32_t ready = poll_args[i].revents;
            if (ready == 0) {
                continue;
            }
            std::unique_ptr<Connection> connection = std::move(fd_to_connection[poll_args[i].fd]);
            int fd_index = connection->fd;
            if (ready & POLLIN) {
                auto placeholder = HandleRead(std::move(connection));
                fd_to_connection[fd_index] = std::move(placeholder);
            } else if (ready & POLLOUT) {
                auto placeholder = HandleWrite(std::move(connection));
                fd_to_connection[fd_index] = std::move(placeholder);
            } else if ((ready & POLLERR) || connection->status == ConnectionStatus::CLOSE) {
                close(fd_index);
                // explicitly don't place it back into the fd_to_connection
                // vector it'll destroy when this scope closes
            }
        }
    }
    return 0;
}
