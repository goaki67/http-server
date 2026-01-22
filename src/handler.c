#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "arena.h"
#include "file.h"
#include "handler.h"
#include "http.h"
#include "log.h"
#include "string_utils.h"

void handle_client(arena_t *memory, int client, string_t *root_dir) {
  if (client < 0) {
    return;
  }
  char *buffer = arena_alloc(memory, BUFFER_SIZE);
  (void)read(client, buffer, BUFFER_SIZE - 1);
  buffer[BUFFER_SIZE - 1] = '\0';
  printf("\nMessage recived: \"\n%s\n\"\n", buffer);

  string_t *buf = string_init(nullptr, buffer);

  http_request_t request = parse_http(buf);
  log_trace("%s %s", request.method, request.uri);
  string_t *filename = string_init(nullptr, request.uri);
  string_t *filepath;
  if (filename == nullptr ||
      (filename->length == 1 && filename->data[0] == '/')) {
    filename = string_init(filename, "index.html");
    filepath = get_safe_path(root_dir, filename);
  } else {
    filepath = get_safe_path(root_dir, filename);
  }
  string_destroy(filename);
  string_destroy(buf);
  free(filename);
  free(request.uri);
  free(buf);

  char *message;
  if (filepath != nullptr) {
    log_trace(filepath->data);

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
    string_destroy(filepath);
    free(filepath);
  } else {
    log_error("File not found");
    message = "HTTP/1.0 404 NOT FOUND\n\nFile Not Found";
    (void)write(client, message, strlen(message));
  }

  close(client);
}
