#ifndef SOCKET_H
#define SOCKET_H

#include <stdint.h>

[[nodiscard]]
int open_socket(uint16_t port_number);
[[nodiscard]]
int get_client(int sockfd);

[[nodiscard]]
int parse_port(const char *str, uint16_t *out_port);

#endif // !SOCKET_H
