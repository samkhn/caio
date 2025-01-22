#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static void log_info(const char *msg) {
	fprintf(stderr, "%s\n", msg);
}

static void log_fatal(const char *msg) {
	int err = errno;
	fprintf(stderr, "[%d] %s\n", err, msg);
	abort();
}

// TODO? maybe return the erroring socket for logging
static void respond(int connfd) {
	char rbuf[64] = {0};
	ssize_t n = read(connfd, rbuf, sizeof(rbuf) - 1);
	if (n < 0) {
		log_info("read from socket error");
		return;
	}
	printf("client says %s\n", rbuf);
	char wbuf[] = "world";
	write(connfd, wbuf, strlen(wbuf));
}

int main() {
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0) {
		log_fatal("failed to construct socket");
	}

	int optval = 1;
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

	struct sockaddr_in addr = {0};
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
		struct sockaddr_in client_addr = {0};
		socklen_t socklen = sizeof(client_addr);
		int connfd = accept(fd, (struct sockaddr *)&client_addr, &socklen);
		if (connfd < 0) {
			continue;
		}

		respond(connfd);
		close(connfd);
	}

	return 0;
}
