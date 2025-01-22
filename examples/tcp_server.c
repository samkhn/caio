#include "buffer.h"
#include "logging.h"

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

const size_t k_max_msg = 4096;

static int32_t respond(int connfd)
{
	char rbuf[4 +
		  k_max_msg]; // message is len (4 chars), then content (4096 chars)
	errno = 0; // reset
	int32_t err = read_full(connfd, rbuf, 4);
	if (err) {
		log_info(errno == 0 ? "EOF" :
				      "error in reading len of message");
		return err;
	}
	uint32_t len = 0;
	memcpy(&len, rbuf, 4); // assumes little endian
	if (len > k_max_msg) {
		log_info("len of message too long");
		return -1;
	}
	err = read_full(connfd, &rbuf[4], len);
	if (err) {
		log_info("error in reading message");
		return err;
	}
	printf("client says: %.*s\n", len, &rbuf[4]);

	const char reply[] = "world";
	char wbuf[4 + sizeof(reply)];
	len = (uint32_t)strlen(reply);
	memcpy(wbuf, &len, 4);
	memcpy(&wbuf[4], reply, len);
	return write_all(connfd, wbuf, 4 + len);
}

int main()
{
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0) {
		log_fatal("failed to construct socket");
	}

	int optval = 1;
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

	struct sockaddr_in addr = { 0 };
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = ntohl(0);
	addr.sin_port = ntohs(1234);
	int rv = bind(fd, (const struct sockaddr *)&addr, sizeof(addr));
	if (rv) {
		log_fatal("failed to bind");
	}

	rv = listen(fd, SOMAXCONN);
	if (rv) {
		log_fatal("failed to listen");
	}

	// TODO event loop
	while (1) {
		struct sockaddr_in client_addr = { 0 };
		socklen_t socklen = sizeof(client_addr);
		int connfd =
			accept(fd, (struct sockaddr *)&client_addr, &socklen);
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
