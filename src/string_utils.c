#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "string_utils.h"

[[nodiscard]]
char **str_split(char *input, char delimiter) {
  if (input == nullptr) {
    log_warn("Input is a nullptr");
    return nullptr;
  }
  if (delimiter == '\0') {
    log_warn("Delimiter is \\0");
    return nullptr;
  }

  size_t delimiter_count = 1;
  for (char *ch = input; *ch != '\0'; ch++) {
    if (*ch == delimiter) {
      delimiter_count++;
    }
  }

  /* +1 for the NULL terminator at the end */
  char **result = (char **)malloc(sizeof(char *) * (delimiter_count + 1));
  if (result == nullptr) {
    log_error("While allocating memory: %s", strerror(errno));
    return nullptr;
  }

  size_t index = 0;
  result[index] = input;
  index++;

  for (char *ch = input; *ch; ch++) {
    if (*ch == delimiter) {
      *ch = '\0';
      result[index] = ch + 1;
      index++;
    }
  }

  result[index] = nullptr;

  return result;
}

char *get_line(char **tokens, size_t index) {
  if (tokens == nullptr) {
    return nullptr;
  }

  for (size_t i = 0; i <= index; i++) {
    if (tokens[i] == nullptr) {
      return nullptr;
    }

    if (i == index) {
      return tokens[i];
    }
  }
  return nullptr;
}
