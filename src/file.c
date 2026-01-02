#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "file.h"
#include "log.h"
#include "string_utils.h"

file_t get_file_contents(const char *file_path) {
  file_t data = {.data = nullptr, .length = 0};

  FILE *file = fopen(file_path, "rb");
  if (file == nullptr) {
    log_warn("Problem opening file \"%s\": %s", file_path, strerror(errno));
    return data;
  }

  struct stat path_stat;
  if (fstat(fileno(file), &path_stat) == -1) {
    log_error("Failed to stat \"%s\": %s", file_path, strerror(errno));
    (void)fclose(file);
    return data;
  }

  if (S_ISDIR(path_stat.st_mode)) {
    log_warn("Path \"%s\" is a directory, not a file", file_path);
    (void)fclose(file);
    return data;
  }

  if (!S_ISREG(path_stat.st_mode)) {
    log_warn("Not a regular file: \"%s\"", file_path);
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
      log_error("Read error \"%s\": %s", file_path, strerror(errno));
    } else {
      log_error("Short read \"%s\" (Expected %ld, got %zu)", file_path, length,
                read_length);
    }
    free(data.data);
    (void)fclose(file);
    return (file_t){.data = nullptr, .length = 0};
  }
  data.length = read_length;

  if (fclose(file) == EOF) {
    log_warn("Close failed \"%s\": %s", file_path, strerror(errno));
  }
  return data;
}

[[nodiscard]]
char *get_safe_path(char *root_path, char *file_path) {
  if (root_path == nullptr || file_path == nullptr) {
    log_warn("Got nullptr instead of a string");
    return nullptr;
  }

  char *resolved_root = realpath(root_path, nullptr);
  if (resolved_root == nullptr) {
    log_error("Could not resolve root directory \"%s\": %s", root_path,
              strerror(errno));
    return nullptr;
  }

  size_t len = strlen(resolved_root) + strlen(file_path) + 2;
  char *file = malloc(len);
  (void)snprintf(file, len, "%s/%s", resolved_root, file_path);

  char *resolved_file = realpath(file, nullptr);
  if (resolved_file == nullptr) {
    log_error("Could not resolve file directory \"%s\": %s", file,
              strerror(errno));
    free(file);
    free(resolved_root);
    return nullptr;
  }

  free(file);

  if (str_starts_with(resolved_file, resolved_root)) {
    char boundary_char = resolved_file[strlen(resolved_root)];
    free(resolved_root);

    if (boundary_char != '\0' && boundary_char != '/') {
      free(resolved_file);
      return nullptr;
    }

    return resolved_file;
  }

  free(resolved_file);
  free(resolved_root);
  return nullptr;
}
