#ifndef QUEUE_H
#define QUEUE_H

#include <pthread.h>
#include <stdbool.h>

#define QUEUE_SIZE 256

typedef struct {
  int sockets[QUEUE_SIZE]; // Circular buffer
  int head;
  int tail;
  int count;
  bool shutdown;         // Flag to stop workers
  pthread_mutex_t lock;  // Protects all fields above
  pthread_cond_t notify; // Signals workers when count > 0
} job_queue_t;

void queue_init(job_queue_t *q);
int queue_push(job_queue_t *q, int client_fd);
int queue_pop(job_queue_t *q);
void queue_shutdown(job_queue_t *q);
void queue_destroy(job_queue_t *q);

#endif
