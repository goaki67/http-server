#ifndef HTTP_H
#define HTTP_H

#include "string_utils.h"
[[nodiscard]]
char **http_split_lines(char *http_request);
[[nodiscard]]
string_t *get_filename_from_http(char **http_request);

#endif // !HTTP_H
