#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "arena.h"
#include "log.h"

/*
typedef struct {
  unsigned char *base;
  size_t capacity;
  size_t offset;
} arena_t;
*/

[[nodiscard]]
arena_t *arena_create(size_t size) {
  if (size == 0) {
    log_warn("Cannot allocate arena of size 0.");
    return nullptr;
  }

  if (size > ARENA_MAX_SIZE) {
    log_warn("Cannot allocate arena of size larger than %zu.", ARENA_MAX_SIZE);
    return nullptr;
  }

  arena_t *arena = (arena_t *)calloc(1, sizeof(arena_t));
  if (arena == nullptr) {
    log_warn("OOM cannot allocate memory for the arena, size: %zu", size);
    return nullptr;
  }

  arena->base = (unsigned char *)calloc(size, sizeof(unsigned char));
  if (arena->base == nullptr) {
    log_warn("OOM cannot allocate memory for the arena, size: %zu", size);
    free(arena);
    return nullptr;
  }
  arena->capacity = size;
  arena->offset = 0;

  return arena;
}

[[nodiscard]]
void *arena_alloc(arena_t *a, size_t size) {
  if (a == nullptr) {
    log_warn("Arena is a nullptr.");
    return nullptr;
  }

  uintptr_t physical = (uintptr_t)a->base + a->offset;
  uintptr_t align = sizeof(void *);
  size_t padding = (align - (physical & (align - 1))) & (align - 1);

  if (size == 0 || size + padding > a->capacity - a->offset) {
    log_warn("OOM in the arena.");
    return nullptr;
  }

  a->offset += padding;
  void *ptr = (void *)(a->base + a->offset);
  a->offset += size;

  return ptr;
}

void arena_reset(arena_t *a) {
  if (a == nullptr) {
    log_warn("Cannot reset a null arena.");
    return;
  }

  a->offset = 0;

#ifdef DEBUG
  memset(a->base, 0, a->capacity);
#endif
}

void arena_destroy(arena_t *a) {
  if (a == nullptr) {
    log_warn("Cannot destroy a nuulptr.");
    return;
  }

  if (a->base != nullptr) {
    free(a->base);
  }

  free(a);
}
