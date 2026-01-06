#include <pthread.h>

#include "log.h"
#include "log_config.h"

static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

static void lock_callback(bool lock, void *udata) {
  pthread_mutex_t *LOCK = (pthread_mutex_t *)udata;

  if (lock) {
    pthread_mutex_lock(LOCK);
  } else {
    pthread_mutex_unlock(LOCK);
  }
}

void log_setup(void) {
  log_set_lock(lock_callback, &log_mutex);
  log_info("Logger initialized with thread-safe locking.");
}
