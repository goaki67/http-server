#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <stddef.h>

[[nodiscard]]
char **str_split(char *input, char delimiter);

char *get_line(char **tokens, size_t index);

[[nodiscard]]
bool str_starts_with(const char *str, const char *prefix);

#endif // !STRING_UTILS_H
