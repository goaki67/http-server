#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "http.h"
#include "log.h"
#include "string_utils.h"

char **http_split_lines(char *http_request) {
  char **rtn = str_split(http_request, '\n');
  if (rtn == nullptr) {
    log_warn("Could not split http request: %s", http_request);
    return nullptr;
  }
  return rtn;
}

string_t *get_filename_from_http(char **http_request) {
  char *first_line = get_line(http_request, 0);
  if (first_line == nullptr) {
    log_warn("Empty HTTP request");
    return nullptr;
  }

  char *first_line_tmp = strdup(first_line);
  if (first_line_tmp == nullptr) {
    log_error("Duplicating first line: %s", strerror(errno));
    return nullptr;
  }

  char **tmp = str_split(first_line_tmp, ' ');
  if (tmp == nullptr) {
    free(first_line_tmp);
    return nullptr;
  }

  char *filename_ptr = get_line(tmp, 1);
  string_t *final_filename = nullptr;

  if (filename_ptr != nullptr) {
    final_filename = (string_t *)calloc(1, sizeof(string_t));
    (void)string_init(final_filename, filename_ptr);
  } else {
    log_warn("Malformed Request: No path found in '%s'", first_line);
  }

  free((void *)tmp);
  free(first_line_tmp);

  return final_filename;
}
