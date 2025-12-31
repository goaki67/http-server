#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "file.h"
#include "log.h"

char *get_file_contents(const char *file_path) {
  FILE *file = fopen(file_path, "rb");
  if (file == nullptr) {
    log_warn("Problem opening file \"%s\": %s", file_path, strerror(errno));
    return nullptr;
  }

  if (fseek(file, 0, SEEK_END) < 0) {
    log_error("Error with seek \"%s\": %s", file_path, strerror(errno));
    (void)fclose(file);
    return nullptr;
  }

  long length = ftell(file);
  if (length < 0) {
    log_error("Error getting position \"%s\": %s", file_path, strerror(errno));
    (void)fclose(file);
    return nullptr;
  }

  if (fseek(file, 0, SEEK_SET) < 0) {
    log_error("Error rewinding \"%s\": %s", file_path, strerror(errno));
    (void)fclose(file);
    return nullptr;
  }

  char *contents = malloc((size_t)length + 1);
  if (contents == nullptr) {
    (void)fclose(file);
    log_error("While allocating memory: %s", strerror(errno));
    return nullptr;
  }
  size_t read_length = fread(contents, 1, (size_t)length, file);
  if (read_length != (size_t)length) {
    if (ferror(file)) {
      log_error("Read error \"%s\": %s", file_path, strerror(errno));
    } else {
      log_error("Short read \"%s\" (Expected %ld, got %zu)", file_path, length,
                read_length);
    }
    free(contents);
    (void)fclose(file);
    return nullptr;
  }
  contents[length] = '\0';

  if (fclose(file) == EOF) {
    log_warn("Close failed \"%s\": %s", file_path, strerror(errno));
  }
  return contents;
}
