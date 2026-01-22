#ifndef ARENA_H
#define ARENA_H

#include <stddef.h>
#include <stdint.h>

enum {
  ARENA_MAX_SIZE = 64 * 1000,
};

typedef struct {
  unsigned char *base;
  size_t capacity;
  size_t offset;
} arena_t;

[[nodiscard]]
arena_t *arena_create(size_t size);

[[nodiscard]]
void *arena_alloc(arena_t *a, size_t size);

void arena_reset(arena_t *a);
void arena_destroy(arena_t *a);

#endif
