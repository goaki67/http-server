#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "constants.h"
#include "log.h"
#include "socket.h"

int main(int argc, char *argv[]) {
  if (argc < 2) {
    log_fatal("Expected port_number as a second argument");
    return EXIT_FAILURE;
  }

  uint16_t port_number = (uint16_t)atoi(argv[1]);
  int sockfd = open_socket(port_number);
  const char *message = "HTTP/1.0 200 OK\n\n<h1>Hello World</h1>";
  char buffer[BUFFER_SIZE] = {0};

  while (true) {

    int client = get_client(sockfd);
    // read the request from the client into buffer
    (void)read(client, buffer, BUFFER_SIZE - 1);
    buffer[BUFFER_SIZE - 1] = '\0';
    printf("\nMessage recived: \"\n%s\n\"\n", buffer);

    // return the message to the client
    (void)write(client, message, strlen(message));
    close(client);
  }

  close(sockfd);

  return EXIT_SUCCESS;
}
