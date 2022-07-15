#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>
#include <unistd.h>
#include <ctype.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <sys/select.h>
#include <fcntl.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

int sockfd = 0;

void log_msg(const char *, ...);
void cleanup();
void signal_handler(int );
double time_diff(struct timeval, struct timeval);
int tcp_send(int, const char *, int);
int tcp_wait_for_char(int, char);
int tcp_recv(int, char *, int);
int tcp_connect(const char *, const char *);
void tcp_disconnect(int);
int64_t htonll(int64_t);
int64_t ntohll(int64_t);

int main(int argc, char **argv) {
	char *address = "na61db.cern.ch";
	char *port = "54331";
	int i;
	int16_t cmd;
	int32_t channel_id;
	int64_t time_from;
	int64_t time_to;
	int64_t step;
	int32_t val;
	float value;
	char msg[2048] = {0};
	char *ptr = NULL;
	uint16_t size, packet_size, frame_size;
	char client[130] = {0};
	char datetime[20] = {0};
	time_t time_unix;

	if (atexit(cleanup) != 0) {
		log_msg("Register atexit function failed!\n");
		exit(EXIT_FAILURE);
	}

	signal(SIGTERM, signal_handler);
	signal(SIGINT,  signal_handler);
	signal(SIGQUIT, signal_handler);
	signal(SIGHUP,  signal_handler);
	signal(SIGABRT, signal_handler);

	if ((sockfd = tcp_connect(address, port)) <= 0) {
		return 2;
	}

	cmd = 3;
	if (argc > 1) channel_id = atoi(argv[1]);
	else channel_id = 13;
	time_from = 1423638000;
	if (argc > 2) time_from = atol(argv[2]);
	time_to = time_from + 20*3600;
	if (argc > 3) time_to = atol(argv[3]);
	step = 0;
	strcat(client, "OfflineQANominalDriftVelocityDump");
	size = strlen(client);

	frame_size = sizeof(cmd) + sizeof(channel_id) + sizeof(size) + size + sizeof(time_from) + sizeof(time_to) + sizeof(step);

	log_msg("Sending: %d %d %d %d %s %d %d %d\n", frame_size, cmd, channel_id, size, client, time_from, time_to, step);

	cmd = htons(cmd);
	channel_id = htonl(channel_id);
	time_to = htonll(time_to);
	time_from = htonll(time_from);
	step = htonll(step);
	packet_size = htons(size);
	frame_size = htons(frame_size);

	if (tcp_send(sockfd, (char *)&frame_size, 2) < 0) {
		log_msg("Sending frame_size failed\n");
		exit(-1);
	}

	if (tcp_send(sockfd, (char *)&cmd, 2) < 0) {
		log_msg("Sending cmd failed\n");
		exit(-1);
	}

	if (tcp_send(sockfd, (char *)&channel_id, 4) < 0) {
		log_msg("Sending channel_id failed\n");
		exit(-1);
	}

	if (tcp_send(sockfd, (char *)&packet_size, 2) < 0) {
		log_msg("Sending packet_size failed\n");
		exit(-1);
	}

	if (tcp_send(sockfd, client, size) < 0) {
		log_msg("Sending client failed\n");
		exit(-1);
	}

	if (tcp_send(sockfd, (char *)&time_from, 8) < 0) {
		log_msg("Sending time_from failed\n");
		exit(-1);
	}

	if (tcp_send(sockfd, (char *)&time_to, 8) < 0) {
		log_msg("Sending time_to failed\n");
		exit(-1);
	}

	if (tcp_send(sockfd, (char *)&step, 8) < 0) {
		log_msg("Sending step failed\n");
		exit(-1);
	}

	do {
		if (tcp_recv(sockfd, (char *)&frame_size, 2) < 0) {
			log_msg("Receiving frame size failed\n");
			exit(-1);
		}

		frame_size = ntohs(frame_size);

		log_msg("Frame size: %d\n", frame_size);

		if ((i = tcp_recv(sockfd, (char *)&cmd, 2)) < 0) {
			log_msg("Receiving cmd failed\n");
			exit(-1);
		}

		frame_size -= i;
		cmd = ntohs(cmd);

		if ((i = tcp_recv(sockfd, (char *)&channel_id, 4)) < 0) {
			log_msg("Receiving chanel_id failed\n");
			exit(-1);
		}

		frame_size -= i;
		channel_id  = ntohl(channel_id);

		if ((i = tcp_recv(sockfd, (char *)&size, 2)) < 0) {
			log_msg("Receiving size failed\n");
			exit(-1);
		}

		frame_size -= i;
		size = ntohs(size);

		if ((i = tcp_recv(sockfd, client, size)) < 0) {
			log_msg("Receiving client string failed\n");
			exit(-1);
		}

		frame_size -= i;
		log_msg("Received: cmd=%d, chcnnel=%d, str=%s\n", cmd, channel_id, client);

		while (frame_size > 0) {
			if ((i = tcp_recv(sockfd, (char *)&time_from, 8)) < 0) {
				log_msg("Receiving step failed\n");
				exit(-1);
			}

			frame_size -= i;
			time_from = ntohll(time_from);

			if ((i = tcp_recv(sockfd, (char *)&val, 4)) < 0) {
				log_msg("Receiving step failed\n");
				exit(-1);
			}

			frame_size -= i;
			val = ntohl(val);
			value = *(float *)&val;

			time_unix = (time_t)time_from;
			strftime(datetime, sizeof(datetime), "%Y-%m-%d %H:%M:%S", localtime(&time_unix));

			printf("%u\t%f\n", (unsigned int)time_unix, value);
		}

	} while (1);

    return 0;
}

int64_t htonll(int64_t value) {
	uint8_t *a = (uint8_t *)&value;
	int64_t new_value = 0;
	uint8_t *b = (uint8_t *)&new_value;
	int i;

	for (i=0; i<8; ++i) {
		b[i] = a[8-i-1];
	}

	return new_value;
}

int64_t ntohll(int64_t value) {
	uint8_t *a = (uint8_t *)&value;
	int64_t new_value = 0;
	uint8_t *b = (uint8_t *)&new_value;
	int i;

	for (i=0; i<8; ++i) {
		b[i] = a[8-i-1];
	}

	return new_value;
}

void log_msg(const char *format, ...) {
	char buf[512] = {0};
	char msg[512] = {0};
	va_list args;
	struct timeval current_time;
	struct tm timestruct;
	FILE *f = NULL;
	int verbatim_mode = 0;
	int i, size;
	char *padding = "                         \t";

	gettimeofday(&current_time, NULL);
	localtime_r(&current_time.tv_sec, &timestruct);
	size = strftime(msg, sizeof(msg) - 1, "[%Y-%m-%d %H:%M:%S", &timestruct);
	size += sprintf(msg+size, ".%03d]\t", (int)(current_time.tv_usec / 1e3));

	va_start(args, format);
	vsnprintf(buf, sizeof(buf) - 1, format, args);
	va_end(args);

	i = 0;
	while (size < sizeof(msg) - 1) {
		if (buf[i] == '\"') {
			if (verbatim_mode) verbatim_mode = 0;
			else verbatim_mode = 1;
		}

		if (!isprint(buf[i])) {
			if (verbatim_mode) {
				if (buf[i] == '\n') {
					msg[size++] = '\\';
					msg[size++] = 'n';
				} else if (buf[i] == '\r') {
					msg[size++] = '\\';
					msg[size++] = 'r';
				} else if (buf[i] == '\t') {
					msg[size++] = '\\';
					msg[size++] = 't';
				} else {
					size += sprintf(msg + size, "<%02hhx>", buf[i]);
				}
			} else {
				if (buf[i] == '\0') {
					break;
				} else if (buf[i] == '\n') {
					msg[size++] = buf[i];
					if (buf[i+1] != '\0') {
						size += sprintf(msg + size, "%s", padding);
					}
				} else if (buf[i] == '\t') {
					msg[size++] = buf[i];
				}
			}
		} else {
			msg[size++] = buf[i];
		}
		++i;
	}

	fputs(msg, stderr);

	if ((f = fopen("logfile.log", "a")) != NULL) {
		fputs(msg, f);
		fclose(f);
	}

}

void cleanup() {
	if (sockfd) {
		tcp_disconnect(sockfd);
	}
}

void signal_handler(int signum) {
	log_msg("Signal %d (%s) caught.\n", signum, strsignal(signum));

	exit(EXIT_FAILURE);
}

double time_diff(struct timeval begin, struct timeval end) {
	double start = begin.tv_sec + (double)begin.tv_usec/1e6;
	double finish = end.tv_sec + (double)end.tv_usec/1e6;

	return finish - start;
}

int tcp_send(int sockfd, const char *buf, int size) {
	int sent = 0;
	int sent_total = 0;
	int status;
	struct timeval timeout;
	fd_set fds;
	int try, i;
	struct timeval time_begin, time_end;

	gettimeofday(&time_begin, NULL);

	try = 0;
	do {
		sent = send(sockfd, buf + sent_total, size - sent_total, 0);

		if (sent < 0) {
			if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
				timeout.tv_sec = 5;
				timeout.tv_usec = 0;

				FD_ZERO(&fds);
				FD_SET(sockfd, &fds);

				status = select(sockfd + 1, NULL, &fds, NULL, &timeout);

				if (status < 0) {
					log_msg("Error while selecting socket for reading", errno);
					return -1;
				} else if (status == 0) {
					log_msg("Sending timeout (sent %d/%d bytes)\n", sent_total, size);
					return -1;
				} else {
					continue;
				}
			} else {
				log_msg("Error while rceiving [%d: %s]\n", errno, strerror(errno));
				return -1;
			}
		}

		sent_total += sent;
		++try;
	} while (sent_total < size);

	gettimeofday(&time_end, NULL);
/*
	for (i=0; i<size; ++i) {
		printf("<%02d>", (unsigned char)buf[i]);
	}
	printf("\n");
*/
	return sent_total;
}

int tcp_wait_for_char(int sockfd, char c) {
	int total_received = 0, received, status;
	struct timeval timeout;
	fd_set fds;
	struct timeval time_begin, time_end;
	char buf;

	gettimeofday(&time_begin, NULL);

	do {
		received = recv(sockfd, &buf, 1, 0);

		if (received < 0) {
			if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
				timeout.tv_sec = 60;
				timeout.tv_usec = 0;

				FD_ZERO(&fds);
				FD_SET(sockfd, &fds);

				status = select(sockfd + 1, &fds, NULL, NULL, &timeout);
				if (status < 0) {
					log_msg("Error while selecting socket for reading", errno);
					return -1;
				} else if (status == 0) {
					log_msg("Reading timeout (read %d bytes)\n", total_received);
					return -1;
				} else {
					continue;
				}
			} else {
				log_msg("Error while rceiving [%d: %s]\n", errno, strerror(errno));
				return -1;
			}
		}

		total_received += received;
	} while (buf != c);

	gettimeofday(&time_end, NULL);

	return total_received;
}

int tcp_recv(int sockfd, char *buf, int size) {
	int total_received = 0, received, status;
	struct timeval timeout;
	fd_set fds;
	int try;
	struct timeval time_begin, time_end;

	gettimeofday(&time_begin, NULL);

	try = 0;
	do {
		received = recv(sockfd, buf + total_received, size-total_received, 0);

		if (received < 0) {
			if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
				timeout.tv_sec = 1;
				timeout.tv_usec = 0;

				FD_ZERO(&fds);
				FD_SET(sockfd, &fds);

				status = select(sockfd + 1, &fds, NULL, NULL, &timeout);
				if (status < 0) {
					log_msg("Error while selecting socket for reading", errno);
					return -1;
				} else if (status == 0) {
					log_msg("Reading timeout (read %d/%d bytes)\n", total_received, size);
					return -1;
				} else {
					continue;
				}
			} else {
				log_msg("Error while rceiving [%d: %s]\n", errno, strerror(errno));
				return -1;
			}
		}

		total_received += received;
		++try;
	} while (total_received < size);

	gettimeofday(&time_end, NULL);

	return total_received;
}

int tcp_connect(const char *address, const char *port) {
	struct addrinfo *server_address;
	struct addrinfo hints;
	int status, sock;
	fd_set fds;
	int oflag, sockoptval;
	socklen_t sockopt_size;
	struct timeval timeout;
	struct timeval time_begin, time_end;

	gettimeofday(&time_begin, NULL);

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = 0;

	if ((status = getaddrinfo(address, port, &hints, &server_address)) != 0) {
		log_msg("Error on getaddrinfo: %s (%d)\n", gai_strerror(status), status);
		return -1;
	}

	if ((sock = socket(server_address->ai_family, server_address->ai_socktype, server_address->ai_protocol)) == -1) {
		log_msg("Error on socket: %s (%d)\n", strerror(errno), errno);
		freeaddrinfo(server_address);
		return -1;
	}

	oflag = fcntl(sock, F_GETFL, 0);
	oflag |= O_NONBLOCK;
	fcntl(sock, F_SETFL, oflag);

	timeout.tv_sec = 5;
	timeout.tv_usec = 0;

	if (connect(sock, server_address->ai_addr, server_address->ai_addrlen) != 0) {
		if (errno == EINPROGRESS) {
			FD_ZERO(&fds);
			FD_SET(sock, &fds);

			status = select(sock + 1, NULL, &fds, NULL, &timeout);

			if ((status < 0) && (errno != EINTR)) {
				log_msg("Error while selecting the socket: %s (%d).\n", strerror(errno), errno);
				freeaddrinfo(server_address);
				close(sock);
				return -1;
			} else if (status > 0) {
				sockopt_size = sizeof(sockoptval);
				if (getsockopt(sock, SOL_SOCKET, SO_ERROR, (void *)&sockoptval, &sockopt_size) < 0) {
					log_msg("Error while getting socket options: %s (%d).\n", strerror(errno), errno);
					freeaddrinfo(server_address);
					close(sock);
					return -1;
				}
				if (sockoptval) {
					log_msg("Error during connection: %s (%d).\n", strerror(sockoptval), sockoptval);
					freeaddrinfo(server_address);
					close(sock);
					return -1;
				}
			} else {
				log_msg("Connection timeout\n");
				freeaddrinfo(server_address);
				close(sock);
				return -1;
			}
		} else {
			log_msg("Error connecting to socket: %s (%d).\n", strerror(errno), errno);
			freeaddrinfo(server_address);
			close(sock);
			return -1;
		}
	}

	freeaddrinfo(server_address);

	sockoptval = 1;
	setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &sockoptval, sizeof(sockoptval));

	gettimeofday(&time_end, NULL);

	log_msg("Connected to %s:%s in %.3fms.\n", address, port, time_diff(time_begin, time_end)*1e3);

	return sock;
}

void tcp_disconnect(int sock) {
	shutdown(sock, SHUT_RDWR);
	close(sock);
	sockfd = 0;

	log_msg("Disconnected.\n");
}
