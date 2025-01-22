#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include <string_view>

#include "buffer.hpp"
#include "logging.hpp"

static constexpr size_t kMaxMessageSize = 4096;
static constexpr std::string_view reply = "world";

static int32_t respond(int connfd) {
  char read_buffer[4 + kMaxMessageSize];  // message is length (4 chars), then
                                          // content (4096 chars)
  errno = 0;                              // reset
  int32_t err = read_full(connfd, read_buffer, 4);
  if (err) {
    log_info(errno == 0 ? "EOF" : "error in reading length of message");
    return err;
  }
  uint32_t length = 0;
  memcpy(&length, read_buffer, 4);  // assumes little endian
  if (length > kMaxMessageSize) {
    log_info("length of message too long");
    return -1;
  }
  err = read_full(connfd, &read_buffer[4], length);
  if (err) {
    log_info("error in reading message");
    return err;
  }
  std::string_view got{&read_buffer[4], length};
  std::cout << "client says: " << got << '\n';

  char write_buffer[4 + reply.size()];
  length = reply.size();
  memcpy(write_buffer, &length, 4);
  memcpy(&write_buffer[4], reply.data(), length);
  return write_all(connfd, write_buffer, 4 + length);
}

int main() {
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd < 0) {
    log_fatal("failed to construct socket");
  }

  int optval = 1;
  setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

  sockaddr_in addr = {};
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = ntohl(0);
  addr.sin_port = ntohs(1234);
  int rv = bind(fd, reinterpret_cast<const sockaddr *>(&addr), sizeof(addr));
  if (rv) {
    log_fatal("failed to bind");
  }

  rv = listen(fd, SOMAXCONN);
  if (rv) {
    log_fatal("failed to listen");
  }

  // TODO: event loop
  while (1) {
    sockaddr_in client_addr = {};
    socklen_t socklength = sizeof(client_addr);
    int connfd =
        accept(fd, reinterpret_cast<sockaddr *>(&client_addr), &socklength);
    if (connfd < 0) {
      continue;
    }

    while (1) {
      int32_t err = respond(connfd);
      if (err) {
        break;
      }
    }
    close(connfd);
  }

  return 0;
}
