#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <stdbool.h>
#include <stddef.h>

#include "arena.h"

typedef struct {
  char *data;
  size_t length;
} string_t;

[[nodiscard]]
string_t *string_create(arena_t *a, const char *initial_value);
[[nodiscard]]
string_t *string_create_from_len(arena_t *a, const char *src, size_t len);

[[nodiscard]]
string_t *string_concat(arena_t *a, const string_t *s1, const char *s2);
[[nodiscard]]
string_t *string_concat_s(arena_t *a, const string_t *s1, const string_t *s2);

[[nodiscard]]
bool string_starts_with(const string_t *str, const char *prefix);
[[nodiscard]]
bool string_starts_with_s(const string_t *str, const string_t *prefix);

#endif
