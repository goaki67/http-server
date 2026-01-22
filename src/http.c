#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "http.h"
#include "log.h"
#include "string_utils.h"

char **http_split_lines(char *http_request) {
  char **rtn = str_split(http_request, '\n');
  if (rtn == nullptr) {
    log_warn("Could not split http request: %s", http_request);
    return nullptr;
  }
  return rtn;
}

string_t *get_filename_from_http(char **http_request) {
  char *first_line = get_line(http_request, 0);
  if (first_line == nullptr) {
    log_warn("Empty HTTP request");
    return nullptr;
  }

  char *first_line_tmp = strdup(first_line);
  if (first_line_tmp == nullptr) {
    log_error("Duplicating first line: %s", strerror(errno));
    return nullptr;
  }

  char **tmp = str_split(first_line_tmp, ' ');
  if (tmp == nullptr) {
    free(first_line_tmp);
    return nullptr;
  }

  char *filename_ptr = get_line(tmp, 1);
  string_t *final_filename = nullptr;

  if (filename_ptr != nullptr) {
    final_filename = (string_t *)calloc(1, sizeof(string_t));
    (void)string_init(final_filename, filename_ptr);
  } else {
    log_warn("Malformed Request: No path found in '%s'", first_line);
  }

  free((void *)tmp);
  free(first_line_tmp);

  return final_filename;
}

http_request_t parse_http(string_t *data) {
  state_enum state = STATE_METHOD;
  http_request_t request = {};

  char *token_start = data->data;
  for (size_t i = 0; i < data->length; i++) {
    char current = data->data[i];

    switch (state) {
    case STATE_METHOD:
      if (current == ' ') {
        request.method_len = (size_t)((data->data + i) - token_start);
        if (request.method_len > MAX_METHOD) {
          state = STATE_ERROR;
          break;
        }
        (void)snprintf(request.method, request.method_len + 1, "%s",
                       token_start);
        state = STATE_SPACE_BEFORE_URI;
      }
      break;
    case STATE_SPACE_BEFORE_URI:
      if (current != ' ') {
        token_start = data->data + i;
        state = STATE_URI;
      }
      break;
    case STATE_URI:
      if (current == ' ') {
        request.uri_len = (size_t)((data->data + i) - token_start);
        if (request.uri_len > MAX_URI) {
          state = STATE_ERROR;
          break;
        }
        request.uri = (char *)malloc(request.uri_len + 1);
        (void)snprintf(request.uri, request.uri_len + 1, "%s", token_start);
        state = STATE_SPACE_BEFORE_VERSION;
      }
      break;

    case STATE_SPACE_BEFORE_VERSION:
      if (current != ' ') {
        state = STATE_VERSION;
      }
      break;

    case STATE_VERSION:
      if (current == '\r') {
        state = STATE_CRLF;
      }
      break;
    case STATE_CRLF:
      if (current == '\n') {
        token_start = data->data + i + 1;
        state = STATE_DONE;
      }
      break;
    case STATE_DONE:
      return request;
    case STATE_ERROR:
      if (request.uri != nullptr) {
        free(request.uri);
      }
      return (http_request_t){};
    default:
      break;
    }
  }

  return request;
}
