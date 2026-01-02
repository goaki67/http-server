#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "constants.h"
#include "file.h"
#include "http.h"
#include "log.h"
#include "socket.h"

int main(int argc, char *argv[]) {
  if (argc < 3) {
    log_fatal("Expected port_number and html_path as arguments");
    return EXIT_FAILURE;
  }

  uint16_t port_number = (uint16_t)atoi(argv[1]);
  int sockfd = open_socket(port_number);
  char buffer[BUFFER_SIZE] = {0};

  while (true) {
    int client = get_client(sockfd);

    (void)read(client, buffer, BUFFER_SIZE - 1);
    buffer[BUFFER_SIZE - 1] = '\0';
    printf("\nMessage recived: \"\n%s\n\"\n", buffer);

    char **http_request = http_split_lines(buffer);
    char *filename = get_filename_from_http(http_request);
    char *filepath;
    if (strlen(filename) == 1 && filename[0] == '/') {
      free(filename);
      filename = "index.html";
      filepath = get_safe_path(argv[2], filename);
    } else {
      filepath = get_safe_path(argv[2], filename);
      free(filename);
    }

    char *message;
    if (filepath != nullptr) {
      log_trace(filepath);
      free((void *)http_request);

      file_t file = get_file_contents(filepath);
      if (file.data == nullptr) {
        log_error("File not found");
        message = "HTTP/1.0 404 NOT FOUND\n\nFile Not Found";
        (void)write(client, message, strlen(message));
      } else {
        char *header = "HTTP/1.0 200 OK\n\n";
        (void)write(client, header, strlen(header));
        (void)write(client, file.data, file.length);
      }
      free(file.data);
      free(filepath);
    } else {
      log_error("File not found");
      message = "HTTP/1.0 404 NOT FOUND\n\nFile Not Found";
      (void)write(client, message, strlen(message));
    }

    close(client);
  }

  close(sockfd);

  return EXIT_SUCCESS;
}
