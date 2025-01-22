#include "buffer.hpp"
#include "logging.hpp"

#include <iostream>
#include <string_view>

#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

static constexpr size_t kMaxMessageSize = 4096;

int32_t query(int fd, std::string_view message) {
  uint32_t length = message.length();
  if (length > kMaxMessageSize) {
    return -1;
  }

  // send request
  char write_buffer[4 + kMaxMessageSize];
  memcpy(write_buffer, &length, 4);
  memcpy(&write_buffer[4], message.data(), length);
  int32_t err = write_all(fd, write_buffer, 4 + length);
  if (err) {
    return err;
  }

  // read header/length
  char read_buffer[4 + kMaxMessageSize + 1];
  errno = 0;
  err = read_full(fd, read_buffer, 4);
  if (err) {
    log_info(errno == 0 ? "EOF" : "error in reading length");
    return err;
  }
  memcpy(&length, read_buffer, 4);
  if (length > kMaxMessageSize) {
    log_info("message too long");
    return -1;
  }

  // read message
  err = read_full(fd, &read_buffer[4], length);
  if (err) {
    log_info("error in reading message");
    return err;
  }

  std::string_view got{&read_buffer[4], length};
  std::cout << "server says: " << got << '\n';
  return 0;
}

int main() {
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd < 0) {
    log_fatal("failed to construct socket");
  }

  struct sockaddr_in addr = {0};
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = ntohl(INADDR_LOOPBACK);
  addr.sin_port = ntohs(1234);
  int dial = connect(fd, (const struct sockaddr *)&addr, sizeof(addr));
  if (dial) {
    log_fatal("failed to connect");
  }

  int32_t err = query(fd, "hello1");
  if (err) {
    goto L_DONE;
  }
  err = query(fd, "hello2");
  if (err) {
    goto L_DONE;
  }

L_DONE:
  close(fd);
  return 0;
}
