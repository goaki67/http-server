#ifndef HANDLER_H
#define HANDLER_H

#include "arena.h"
#include "string_utils.h"

void handle_client(arena_t *memory, int client, string_t *root_dir);

#endif // !HANDLER_H
