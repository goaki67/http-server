#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arena.h"
#include "file.h"
#include "http.h"
#include "string_utils.h"

http_request_t *parse_http(arena_t *memory, string_t *data) {
  state_enum state = STATE_METHOD;
  http_request_t *request =
      (http_request_t *)arena_alloc(memory, sizeof(http_request_t));

  char *token_start = data->data;
  for (size_t i = 0; i < data->length; i++) {
    char current = data->data[i];

    switch (state) {
    case STATE_METHOD:
      if (current == ' ') {
        if ((size_t)((data->data + i) - token_start) > MAX_METHOD) {
          state = STATE_ERROR;
          break;
        }
        request->method = string_create_from_len(
            memory, token_start, (size_t)((data->data + i) - token_start));
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
        if ((size_t)((data->data + i) - token_start) > MAX_URI) {
          state = STATE_ERROR;
          break;
        }
        request->uri = string_create_from_len(
            memory, token_start, (size_t)((data->data + i) - token_start));
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
      return nullptr;
    }
  }

  return request;
}
