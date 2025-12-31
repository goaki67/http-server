#ifndef SOCKET_H
#define SOCKET_H

#include <stdint.h>

[[nodiscard]]
int open_socket(uint16_t port_number);

[[nodiscard]]
int get_client(int sockfd);

#endif // !SOCKET_H
