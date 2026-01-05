#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include "log.h"
#include "queue.h"
#include "socket.h"
#include "string_utils.h"
#include "thread_pool.h"

static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;
volatile sig_atomic_t server_running = 1;

void sig_handler(int signo) {
  (void)signo;
  server_running = 0;
}

void thread_lock_callback(bool lock, void *udata) {
  pthread_mutex_t *LOCK = (pthread_mutex_t *)udata;
  if (lock) {
    pthread_mutex_lock(LOCK);
  } else {
    pthread_mutex_unlock(LOCK);
  }
}

int main(int argc, char *argv[]) {
  log_set_lock(thread_lock_callback, &log_mutex);
  if (argc < 3) {
    log_fatal("Expected port_number and html_path as arguments");
    return EXIT_FAILURE;
  }

  uint16_t port_number;
  if (parse_port(argv[1], &port_number) != 0) {
    log_fatal("Expected port number in the second argument");
    return EXIT_FAILURE;
  }

  string_t root_dir;
  (void)string_init(&root_dir, argv[2]);

  struct sigaction sa = {0};
  sa.sa_handler = sig_handler;
  sigaction(SIGINT, &sa, nullptr);
  sigaction(SIGTERM, &sa, nullptr);

  int sockfd = open_socket(port_number);
  job_queue_t queue;
  queue_init(&queue);

  thread_pool_t pool;
  if (thread_pool_init(&pool, &queue, &root_dir) != 0) {
    return EXIT_FAILURE;
  }

  log_info("Server accepting connections...");
  while (server_running) {
    int client = get_client(sockfd);
    if (client >= 0) {
      queue_push(&queue, client);
    }
  }

  log_info("Stopping server...");

  thread_pool_wait(&pool);
  queue_destroy(&queue);
  close(sockfd);

  return EXIT_SUCCESS;
}
