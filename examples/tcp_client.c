#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#include "buffer.h"
#include "logging.h"

const size_t k_max_msg = 4096;

int32_t query(int fd, const char *message)
{
	uint32_t len = (uint32_t)strlen(message);
	if (len > k_max_msg) {
		return -1;
	}

	// send request
	char wbuf[4 + k_max_msg];
	memcpy(wbuf, &len, 4);
	memcpy(&wbuf[4], message, len);
	int32_t err = write_all(fd, wbuf, 4 + len);
	if (err) {
		return err;
	}

	// read header/len
	char rbuf[4 + k_max_msg + 1];
	errno = 0;
	err = read_full(fd, rbuf, 4);
	if (err) {
		log_info(errno == 0 ? "EOF" : "error in reading len");
		return err;
	}
	memcpy(&len, rbuf, 4);
	if (len > k_max_msg) {
		log_info("message too long");
		return -1;
	}

	// read message
	err = read_full(fd, &rbuf[4], len);
	if (err) {
		log_info("error in reading message");
		return err;
	}

	printf("server says: %.*s\n", len, &rbuf[4]);
	return 0;
}

int main()
{
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0) {
		log_fatal("failed to construct socket");
	}

	struct sockaddr_in addr = { 0 };
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
