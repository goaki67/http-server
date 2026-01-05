#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <stddef.h>
#include <stdlib.h>

#define BUFFER_SIZE 1024

typedef struct {
  size_t length;
  size_t allocated;
  char *data;
} string_t;

// gets a char* and makes a string_t
[[nodiscard]]
int string_init(string_t *str, const char *data);
// free
void string_destroy(string_t *);
// addes the char* to the end of string_t
[[nodiscard]]
int string_append(string_t *, const char *);
[[nodiscard]]
int string_append_s(string_t *, string_t *);
// check if string_t* strats with the second string_t*
[[nodiscard]]
bool string_starts_with(string_t *, char *);
[[nodiscard]]
bool string_starts_with_s(string_t *, string_t *);

[[nodiscard]]
char **str_split(char *input, char delimiter);

char *get_line(char **tokens, size_t index);

#endif // !STRING_UTILS_H
