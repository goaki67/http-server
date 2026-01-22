#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "string_utils.h"

[[nodiscard]]
string_t *string_init(string_t *str, const char *data) {
  if (str == nullptr) {
    str = (string_t *)calloc(1, sizeof(string_t));
  }
  if (str->allocated != 0) {
    string_destroy(str);
  }

  size_t len = data ? strlen(data) : 0;

  if (len == SIZE_MAX) {
    return nullptr;
  }

  str->allocated = len + 1;
  str->data = (char *)malloc(str->allocated);
  if (str->data == nullptr) {
    return nullptr;
  }

  if (data) {
    memcpy(str->data, data, len);
  }
  str->data[len] = '\0';
  str->length = len;
  return str;
}

void string_destroy(string_t *str) {
  if (str == nullptr) {
    return;
  }

  free(str->data);
  str->data = nullptr;
  str->length = 0;
  str->allocated = 0;
}

int string_append(string_t *str, const char *data) {
  if (str == nullptr || data == nullptr) {
    return -EINVAL;
  }

  size_t data_len = strlen(data);

  if (SIZE_MAX - str->length < data_len) {
    return -EOVERFLOW;
  }

  size_t required_len = str->length + data_len;

  if (required_len + 1 > str->allocated) {
    size_t new_cap = str->allocated * 2;

    if (new_cap < required_len + 1) {
      new_cap = required_len + 1;
    }

    if (new_cap < str->allocated) {
      new_cap = SIZE_MAX;
    }

    char *new_data = (char *)realloc(str->data, new_cap);
    if (new_data == nullptr) {
      log_error("While allocating memory: %s", strerror(errno));
      return -ENOMEM;
    }
    str->data = new_data;
    str->allocated = new_cap;
  }

  memcpy(str->data + str->length, data, data_len);
  str->length = required_len;
  str->data[str->length] = '\0';

  return 0;
}

int string_append_s(string_t *str, string_t *data) {
  if (str == nullptr || data == nullptr) {
    return -EINVAL;
  }

  return string_append(str, data->data);
}

bool string_starts_with(string_t *str, char *prefix) {
  if (str == nullptr || prefix == nullptr) {
    return false;
  }

  size_t prefix_len = strlen(prefix);
  if (prefix_len > str->length) {
    return false;
  }

  return strncmp(str->data, prefix, prefix_len) == 0;
}

bool string_starts_with_s(string_t *str, string_t *prefix) {
  if (str == nullptr || prefix == nullptr) {
    return false;
  }

  if (prefix->length > str->length) {
    return false;
  }

  return strncmp(str->data, prefix->data, prefix->length) == 0;
}

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

[[nodiscard]]
bool str_starts_with(const char *str, const char *prefix) {
  if (str == nullptr || prefix == nullptr) {
    return false;
  }

  size_t prefix_len = strlen(prefix);
  size_t str_len = strlen(str);

  if (prefix_len > str_len) {
    return false;
  }

  return strncmp(str, prefix, prefix_len) == 0;
}
