#ifndef HTTP_H
#define HTTP_H

#include <stddef.h>

#include "arena.h"
#include "string_utils.h"

typedef struct {
  string_t *method;
  string_t *uri;
} http_request_t;

http_request_t *parse_http(arena_t *memory, string_t *data);
string_t *http_read_header(arena_t *memory, int sockfd);

#endif // !HTTP_H
