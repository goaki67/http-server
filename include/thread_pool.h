#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include "queue.h"
#include "string_utils.h"
#include <pthread.h>

#define THREAD_POOL_SIZE 4

typedef struct {
  int id;
  job_queue_t *queue;
  string_t *root_dir;
} worker_config_t;

typedef struct {
  pthread_t threads[THREAD_POOL_SIZE];
  worker_config_t configs[THREAD_POOL_SIZE];
} thread_pool_t;

[[nodiscard]]
int thread_pool_init(thread_pool_t *pool, job_queue_t *queue,
                     string_t *root_dir);

void thread_pool_wait(thread_pool_t *pool);

#endif
