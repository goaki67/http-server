#include "thread_pool.h"
#include "handler.h"
#include "log.h"

void thread_lock_callback(bool lock, void *udata) {
  pthread_mutex_t *LOCK = (pthread_mutex_t *)udata;
  if (lock) {
    pthread_mutex_lock(LOCK);
  } else {
    pthread_mutex_unlock(LOCK);
  }
}

static void *worker_entry(void *arg) {
  worker_config_t *cfg = (worker_config_t *)arg;
  log_trace("Worker %d: Online", cfg->id);

  while (true) {
    int client_fd = queue_pop(cfg->queue);

    if (client_fd == -1) {
      log_trace("Worker %d: Shutting down", cfg->id);
      break;
    }

    handle_client(client_fd, cfg->root_dir);
  }

  return nullptr;
}

int thread_pool_init(thread_pool_t *pool, job_queue_t *queue,
                     string_t *root_dir) {
  if (!pool || !queue) {
    return -1;
  }

  for (int i = 0; i < THREAD_POOL_SIZE; i++) {
    pool->configs[i].id = i;
    pool->configs[i].queue = queue;
    pool->configs[i].root_dir = root_dir;

    if (pthread_create(&pool->threads[i], nullptr, worker_entry,
                       &pool->configs[i]) != 0) {
      log_error("Failed to spawn thread %d", i);
      return -1;
    }
  }

  log_info("Thread pool initialized with %d workers", THREAD_POOL_SIZE);
  return 0;
}

void thread_pool_wait(thread_pool_t *pool) {
  if (!pool) {
    return;
  }
  for (int i = 0; i < THREAD_POOL_SIZE; i++) {
    pthread_join(pool->threads[i], nullptr);
  }
}
