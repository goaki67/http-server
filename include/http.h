#ifndef HTTP_H
#define HTTP_H

[[nodiscard]]
char **http_split_lines(char *http_request);
[[nodiscard]]
char *get_filename_from_http(char **http_request);

#endif // !HTTP_H
