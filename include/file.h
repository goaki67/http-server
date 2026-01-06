#ifndef FILE_H
#define FILE_H

#include <stddef.h>
#include <stdint.h>

#include "string_utils.h"

typedef struct file_t {
  uint8_t *data;
  size_t length;
} file_t;

[[nodiscard]]
file_t get_file_contents(const string_t *file_path);

[[nodiscard]]
string_t *get_safe_path(string_t *root_path, string_t *file_path);

typedef enum {
  PATH_ERROR = -1,
  PATH_FILE = 0,
  PATH_DIR = 1,
  PATH_OTHER = 2,
} path_type_t;

path_type_t get_path_type(const string_t *path);

#endif // !FILE_H
