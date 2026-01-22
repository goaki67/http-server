#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "http.h"
#include "log.h"
#include "string_utils.h"

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
