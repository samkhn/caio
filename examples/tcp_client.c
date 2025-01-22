#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static const char* payload = "hello";

static void log_abort(const char *msg) {
	int err = errno;
	fprintf(stderr, "[%d] %s\n", err, msg);
	abort();
}

int main() {
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0) {
		log_abort("failed to construct socket");
	}

	struct sockaddr_in addr = {};
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = ntohl(INADDR_LOOPBACK);
	addr.sin_port = ntohs(1234);
	int dial = connect(fd, (const struct sockaddr *)&addr, sizeof(addr));
	if (dial) {
		log_abort("failed to connect");
	}

	write(fd, payload, strlen(payload));

	char rbuf[64] = {};
	ssize_t n = read(fd, rbuf, sizeof(rbuf) - 1);
	if (n < 0) {
		log_abort("failed to read");
	}

	printf("server said: %s\n", rbuf);
	close(fd);
	return 0;
}
