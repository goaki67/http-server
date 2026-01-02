#include <errno.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "constants.h"
#include "log.h"
#include "socket.h"
/*
  int newsockfd; socklen_t clilen; ssize_t n;
  struct sockaddr_in cli_addr;
*/
int open_socket(uint16_t port_number) {
  int sockfd;
  struct sockaddr_in serv_addr;

  // opens a socket connection
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    close(sockfd);
    log_error("While opening socket: %s", strerror(errno));
    exit(errno);
  }

  int opt = 1;
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
    log_warn("setsockopt error: %s", strerror(errno));
    close(sockfd);
    exit(EXIT_FAILURE);
  }

  memset((char *)&serv_addr, 0, sizeof serv_addr);
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(port_number);
  serv_addr.sin_addr.s_addr = INADDR_ANY;

  if (bind(sockfd, (const struct sockaddr *)&serv_addr, sizeof(serv_addr)) <
      0) {
    close(sockfd);
    log_error("While binding socket: %s", strerror(errno));
    exit(errno);
  }

  if (listen(sockfd, BACKLOG) != 0) {
    close(sockfd);
    log_error("While listening: %s", strerror(errno));
    exit(errno);
  }

  return sockfd;
}

int get_client(int sockfd) {
  int newsockfd;
  socklen_t clilen;
  struct sockaddr_in cli_addr;

  clilen = sizeof(cli_addr);
  newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
  if (newsockfd < 0) {
    close(sockfd);
    close(newsockfd);
    log_error("While accepting a connection: %s", strerror(errno));
    exit(errno);
  }

  return newsockfd;
}
