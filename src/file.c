#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

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

file_t get_file_contents(const string_t *file_path) {
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

  /*
  if (fseek(file, 0, SEEK_END) < 0) {
    log_error("Error with seek \"%s\": %s", file_path, strerror(errno));
    (void)fclose(file);
    return data;
  }

  long length = ftell(file);
  if (length < 0) {
    log_error("Error getting position \"%s\": %s", file_path, strerror(errno));
    (void)fclose(file);
    return data;
  }

  if (fseek(file, 0, SEEK_SET) < 0) {
    log_error("Error rewinding \"%s\": %s", file_path, strerror(errno));
    (void)fclose(file);
    return data;
  }
  */

  if (length == 0) {
    data.data = (uint8_t *)strdup("");
    data.length = 0;
    (void)fclose(file);
    return data;
  }

  data.data = malloc(length);
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
    free(data.data);
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
string_t *get_safe_path(string_t *root_path, string_t *file_path) {
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

  string_t resolved_root;
  if (string_init(&resolved_root, real_root_raw) == nullptr) {
    free(real_root_raw);
    return nullptr;
  }
  free(real_root_raw);

  string_t file;
  if (string_init(&file, resolved_root.data) == nullptr) {
    string_destroy(&resolved_root);
    return nullptr;
  }
  (void)string_append(&file, "/");
  (void)string_append_s(&file, file_path);

  string_t *resolved_file = (string_t *)calloc(1, sizeof(string_t));
  if (resolved_file == nullptr) {
    string_destroy(&file);
    string_destroy(&resolved_root);
    return nullptr;
  }

  char *real_file_raw = realpath(file.data, nullptr);
  if (real_file_raw == nullptr) {
    log_error("Could not resolve file directory \"%s\": %s", file.data,
              strerror(errno));
    string_destroy(&file);
    string_destroy(&resolved_root);
    free(resolved_file);
    return nullptr;
  }

  if (string_init(resolved_file, real_file_raw) == nullptr) {
    free(real_file_raw);
    string_destroy(&file);
    string_destroy(&resolved_root);
    free(resolved_file);
    return nullptr;
  }
  free(real_file_raw);
  string_destroy(&file);

  if (string_starts_with_s(resolved_file, &resolved_root)) {
    char boundary_char = resolved_file->data[resolved_root.length];
    string_destroy(&resolved_root);

    if (boundary_char != '\0' && boundary_char != '/') {
      string_destroy(resolved_file);
      free(resolved_file);
      return nullptr;
    }
    return resolved_file;
  }

  string_destroy(resolved_file);
  free(resolved_file);
  string_destroy(&resolved_root);
  return nullptr;
}
