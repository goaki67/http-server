#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "arena.h"
#include "file.h"
#include "log.h"
#include "string_utils.h"

path_type_t get_path_type(const string_t *path) {
  struct stat path_stat;
  if (stat(path->data, &path_stat) == -1) {
    log_error("Failed to stat \"%s\": %s", path->data, strerror(errno));
    return PATH_ERROR;
  }

  if (S_ISREG(path_stat.st_mode)) {
    return PATH_FILE;
  } else if (S_ISDIR(path_stat.st_mode)) {
    return PATH_DIR;
  }

  return PATH_OTHER;
}

file_t get_file_contents(arena_t *memory, const string_t *file_path) {
  file_t data = {.data = nullptr, .length = 0};

  FILE *file = fopen(file_path->data, "rb");
  if (file == nullptr) {
    log_warn("Problem opening file \"%s\": %s", file_path->data,
             strerror(errno));
    return data;
  }

  struct stat path_stat;
  if (fstat(fileno(file), &path_stat) == -1) {
    log_error("Failed to stat \"%s\": %s", file_path->data, strerror(errno));
    (void)fclose(file);
    return data;
  }

  if (S_ISDIR(path_stat.st_mode)) {
    log_warn("Path \"%s\" is a directory, not a file", file_path->data);
    (void)fclose(file);
    return data;
  }

  if (!S_ISREG(path_stat.st_mode)) {
    log_warn("Not a regular file: \"%s\"", file_path->data);
    (void)fclose(file);
    return data;
  }

  size_t length = (size_t)path_stat.st_size;

  if (length == 0) {
    data.data = (uint8_t *)arena_alloc(memory, 1);
    if (data.data) {
      data.data[0] = '\0';
      data.length = 0;
    }
    (void)fclose(file);
    return data;
  }

  data.data = arena_alloc(memory, length);
  if (data.data == nullptr) {
    (void)fclose(file);
    log_error("While allocating memory: %s", strerror(errno));
    return (file_t){.data = nullptr, .length = 0};
  }

  size_t read_length = fread(data.data, 1, length, file);
  if (read_length != length) {
    if (ferror(file)) {
      log_error("Read error \"%s\": %s", file_path->data, strerror(errno));
    } else {
      log_error("Short read \"%s\" (Expected %ld, got %zu)", file_path->data,
                length, read_length);
    }
    (void)fclose(file);
    return (file_t){.data = nullptr, .length = 0};
  }
  data.length = read_length;

  if (fclose(file) == EOF) {
    log_warn("Close failed \"%s\": %s", file_path->data, strerror(errno));
  }
  return data;
}

[[nodiscard]]
string_t *get_safe_path(arena_t *memory, string_t *root_path,
                        string_t *file_path) {
  if (root_path == nullptr || file_path == nullptr ||
      root_path->data == nullptr || file_path->data == nullptr) {
    log_warn("Got nullptr instead of a string");
    return nullptr;
  }

  char *real_root_raw = realpath(root_path->data, nullptr);
  if (real_root_raw == nullptr) {
    log_error("Could not resolve root directory: %s", strerror(errno));
    return nullptr;
  }

  string_t *resolved_root = string_create(memory, real_root_raw);
  free(real_root_raw);
  if (resolved_root == nullptr) {
    return nullptr;
  }

  string_t *file = string_create(memory, resolved_root->data);
  if (file == nullptr) {
    return nullptr;
  }
  file = string_concat(memory, file, "/");
  file = string_concat_s(memory, file, file_path);

  char *real_file_raw = realpath(file->data, nullptr);
  if (real_file_raw == nullptr) {
    log_error("Could not resolve file directory \"%s\": %s", file->data,
              strerror(errno));
    return nullptr;
  }

  string_t *resolved_file = string_create(memory, real_file_raw);
  free(real_file_raw);
  if (resolved_file == nullptr) {
    return nullptr;
  }

  if (string_starts_with_s(resolved_file, resolved_root)) {
    char boundary_char = resolved_file->data[resolved_root->length];
    if (boundary_char != '\0' && boundary_char != '/') {
      return nullptr;
    }
    return resolved_file;
  }

  return nullptr;
}
