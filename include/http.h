#ifndef HTTP_H
#define HTTP_H

#include <stddef.h>

#include "string_utils.h"

typedef enum {
  MAX_METHOD = 7,
  MAX_URI = 2048,
} limits_enum;

typedef enum {
  STATE_METHOD,
  STATE_SPACE_BEFORE_URI,
  STATE_URI,
  STATE_SPACE_BEFORE_VERSION,
  STATE_VERSION,
  STATE_CRLF,
  STATE_DONE,
  STATE_ERROR,
} state_enum;

/* should free uri */
typedef struct {
  char method[MAX_METHOD];
  size_t method_len;
  char *uri;
  size_t uri_len;
} http_request_t;

http_request_t parse_http(string_t *data);

#endif // !HTTP_H
