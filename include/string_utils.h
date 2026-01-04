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
int string_init(string_t *, char *);
// free
void string_destroy(string_t *);
// addes the char* to the end of string_t
int string_append(string_t *, char *);
int string_append_s(string_t *, string_t *);
// check if string_t* strats with the second string_t*
bool string_starts_with(string_t *, char *);
bool string_starts_with_s(string_t *, string_t *);

[[nodiscard]]
char **str_split(char *input, char delimiter);

char *get_line(char **tokens, size_t index);

[[nodiscard]]
bool str_starts_with(const char *str, const char *prefix);

#endif // !STRING_UTILS_H
