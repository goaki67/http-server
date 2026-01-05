#include "queue.h"

#include <pthread.h>

void queue_init(job_queue_t *q) {
  q->head = 0;
  q->tail = 0;
  q->count = 0;
  q->shutdown = false;
  pthread_mutex_init(&q->lock, nullptr);
  pthread_cond_init(&q->notify, nullptr);
}

int queue_push(job_queue_t *q, int client_fd) {
  pthread_mutex_lock(&q->lock);

  if (q->count < QUEUE_SIZE) {
    q->sockets[q->tail] = client_fd;
    q->tail = (q->tail + 1) % QUEUE_SIZE;
    q->count += 1;

    pthread_cond_signal(&q->notify);
    pthread_mutex_unlock(&q->lock);
    return 0;
  }

  pthread_mutex_unlock(&q->lock);
  return -1;
}

int queue_pop(job_queue_t *q) {
  pthread_mutex_lock(&q->lock);

  while (q->count == 0 && !q->shutdown) {
    pthread_cond_wait(&q->notify, &q->lock);
  }

  if (q->shutdown) {
    pthread_mutex_unlock(&q->lock);
    return -1;
  }

  int client_fd = q->sockets[q->head];
  q->head = (q->head + 1) % QUEUE_SIZE;
  q->count--;

  pthread_mutex_unlock(&q->lock);
  return client_fd;
}

void queue_destroy(job_queue_t *q) {
  if (!q) {
    return;
  }
  pthread_mutex_lock(&q->lock);
  q->shutdown = true;
  pthread_mutex_unlock(&q->lock);

  pthread_cond_broadcast(&q->notify);
}
