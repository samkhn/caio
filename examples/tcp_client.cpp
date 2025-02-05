#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#include <iostream>
#include <string_view>

#include "buffer.hpp"
#include "logging.hpp"

static constexpr std::size_t kMaxHeaderSize = 4;
static constexpr std::size_t kMaxMessageSize = 4096;

static constexpr std::size_t kWriteBufferSize =
    kMaxHeaderSize + kMaxMessageSize;
static constexpr std::size_t kReadBufferSize =
    kMaxHeaderSize + kMaxMessageSize + 1;

namespace {

int Query(int fd, std::string_view message) {
  uint32_t length = message.length();
  if (length > kMaxMessageSize) {
    return -1;
  }

  // send request
  char write_buffer[kWriteBufferSize];
  memcpy(write_buffer, &length, kMaxHeaderSize);
  memcpy(&write_buffer[kMaxHeaderSize], message.data(), length);
  if (int32_t err =
          Net::Buffer::WriteN(fd, write_buffer, kMaxHeaderSize + length)) {
    return err;
  }

  // read header/length
  char read_buffer[kReadBufferSize];
  errno = 0;
  if (int32_t err = Net::Buffer::ReadN(fd, read_buffer, kMaxHeaderSize)) {
    log_info(errno == 0 ? "EOF" : "error in reading length");
    return err;
  }
  memcpy(&length, read_buffer, kMaxHeaderSize);
  if (length > kMaxMessageSize) {
    log_info("message too long");
    return -1;
  }

  // read message
  if (int32_t err =
          Net::Buffer::ReadN(fd, &read_buffer[kMaxHeaderSize], length)) {
    log_info("error in reading message");
    return err;
  }

  std::string_view got{&read_buffer[kMaxHeaderSize], length};
  std::cout << "server says: " << got << '\n';
  return 0;
}

}  // namespace

int main() {
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd < 0) {
    log_fatal("failed to construct socket");
  }

  sockaddr_in addr = {};
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = ntohl(INADDR_LOOPBACK);
  addr.sin_port = ntohs(1234);
  int dial = connect(fd, (const struct sockaddr *)&addr, sizeof(addr));
  if (dial) {
    log_fatal("failed to connect");
  }

  if (Query(fd, "hello1")) {
    goto L_DONE;
  }
  if (Query(fd, "hello2")) {
    goto L_DONE;
  }

L_DONE:
  close(fd);
  return 0;
}
