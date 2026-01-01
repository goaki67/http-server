#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <stddef.h>

[[nodiscard]]
char **str_split(char *input, char delimiter);

char *get_line(char **tokens, size_t index);

#endif // !STRING_UTILS_H
