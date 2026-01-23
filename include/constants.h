#ifndef CONSTANTS_H
#define CONSTANTS_H

enum {
  BUFFER_SIZE = 1024,
  BACKLOG = 1,
};

typedef enum {
  MAX_METHOD = 7,
  MAX_URI = 2048,
} http_limits_enum;

typedef enum {
  STATE_METHOD,
  STATE_SPACE_BEFORE_URI,
  STATE_URI,
  STATE_SPACE_BEFORE_VERSION,
  STATE_VERSION,
  STATE_CRLF,
  STATE_DONE,
  STATE_ERROR,
} http_state_enum;

#endif // !CONSTANTS_H
