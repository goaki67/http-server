#include <stdckdint.h>
#include <stdint.h>
#include <string.h>

#include "arena.h"
#include "log.h"
#include "string_utils.h"

[[nodiscard]]
string_t *string_create_from_len(arena_t *a, const char *src, size_t len) {
  if (a == nullptr) {
    log_error("Attempted to create string with NULL arena.");
    return nullptr;
  }

  string_t *str = (string_t *)arena_alloc(a, sizeof(string_t));
  if (str == nullptr) {
    log_error("Arena OOM: Failed to allocate string_t struct.");
    return nullptr;
  }

  size_t alloc_size = len + 1;

  str->data = (char *)arena_alloc(a, alloc_size);
  if (str->data == nullptr) {
    log_error("Arena OOM: Failed to allocate string buffer (size: %zu).",
              alloc_size);
    return nullptr;
  }

  if (src != nullptr && len > 0) {
    memcpy(str->data, src, len);
  }
  str->data[len] = '\0';
  str->length = len;

  return str;
}

[[nodiscard]]
string_t *string_create(arena_t *a, const char *src) {
  size_t len = 0;
  if (src != nullptr) {
    len = strlen(src);
  }
  return string_create_from_len(a, src, len);
}

[[nodiscard]]
string_t *string_concat(arena_t *a, const string_t *s1, const char *s2) {
  if (a == nullptr) {
    log_error("String concat failed: Arena is NULL.");
    return nullptr;
  }
  if (s1 == nullptr || s2 == nullptr) {
    log_warn("String concat failed: Invalid inputs (s1: %p, s2: %p).",
             (void *)s1, (void *)s2);
    return nullptr;
  }

  size_t len2 = strlen(s2);
  size_t total_len = s1->length + len2;

  string_t *result = string_create_from_len(a, nullptr, total_len);
  if (result == nullptr) {
    log_error("String concat failed: Could not allocate result string.");
    return nullptr;
  }

  if (s1->length > 0) {
    memcpy(result->data, s1->data, s1->length);
  }
  if (len2 > 0) {
    memcpy(result->data + s1->length, s2, len2);
  }

  result->data[total_len] = '\0';
  return result;
}

[[nodiscard]]
string_t *string_concat_s(arena_t *a, const string_t *s1, const string_t *s2) {
  if (a == nullptr) {
    log_error("String concat_s failed: Arena is NULL.");
    return nullptr;
  }
  if (s1 == nullptr || s2 == nullptr) {
    log_warn("String concat_s failed: Invalid inputs (s1: %p, s2: %p).",
             (void *)s1, (void *)s2);
    return nullptr;
  }

  size_t total_len = s1->length + s2->length;
  string_t *result = string_create_from_len(a, nullptr, total_len);
  if (result == nullptr) {
    log_error("String concat_s failed: Could not allocate result string.");
    return nullptr;
  }

  if (s1->length > 0) {
    memcpy(result->data, s1->data, s1->length);
  }
  if (s2->length > 0) {
    memcpy(result->data + s1->length, s2->data, s2->length);
  }

  result->data[total_len] = '\0';
  return result;
}

[[nodiscard]]
bool string_starts_with(const string_t *str, const char *prefix) {
  if (str == nullptr || prefix == nullptr) {
    return false;
  }

  size_t prefix_len = strlen(prefix);
  if (prefix_len > str->length) {
    return false;
  }

  return memcmp(str->data, prefix, prefix_len) == 0;
}

[[nodiscard]]
bool string_starts_with_s(const string_t *str, const string_t *prefix) {
  if (str == nullptr || prefix == nullptr) {
    return false;
  }

  if (prefix->length > str->length) {
    return false;
  }

  return memcmp(str->data, prefix->data, prefix->length) == 0;
}
