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

#define BUFFER_SIZE 1024

void handle_client(arena_t *memory, int client, string_t *root_dir) {
  if (client < 0) {
    return;
  }
  string_t *buffer = http_read_header(memory, client);
  if (buffer == nullptr) {
    close(client);
    return;
  }

  http_request_t *request = parse_http(memory, buffer);
  string_t *filename;
  string_t *filepath;

  if (request->uri->length == 1 && request->uri->data[0] == '/') {
    filename = string_create(memory, "index.html");
    filepath = get_safe_path(memory, root_dir, filename);
  } else {
    filename = request->uri;
    filepath = get_safe_path(memory, root_dir, filename);
  }

  char *message;
  if (filepath != nullptr) {
    log_trace(filepath->data);
    log_trace(filename->data);

    file_t file = get_file_contents(memory, filepath);
    if (file.data == nullptr) {
      log_error("File not found");
      message = "HTTP/1.0 404 NOT FOUND\n\nFile Not Found";
      (void)write(client, message, strlen(message));
    } else {
      char *header = "HTTP/1.0 200 OK\n\n";
      (void)write(client, header, strlen(header));
      (void)write(client, file.data, file.length);
    }
  } else {
    log_error("File not found");
    message = "HTTP/1.0 404 NOT FOUND\n\nFile Not Found";
    (void)write(client, message, strlen(message));
  }

  close(client);
}
