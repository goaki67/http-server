#ifndef FILE_H
#define FILE_H

#include <stddef.h>
#include <stdint.h>

typedef struct file_t {
  uint8_t *data;
  size_t length;
} file_t;

[[nodiscard]]
file_t get_file_contents(const char *file_path);

[[nodiscard]]
char *get_safe_path(char *root_path, char *file_path);

#endif // !FILE_H
