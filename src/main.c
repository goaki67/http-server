#include <pthread.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include "file.h"
#include "log.h"
#include "log_config.h"
#include "queue.h"
#include "sig.h"
#include "socket.h"
#include "string_utils.h"
#include "thread_pool.h"

int setup(int argc, char *argv[], uint16_t *port_number, string_t *root_dir) {
  log_setup();
  if (argc < 3) {
    log_fatal("Usage: %s <port_number> <project_dir>");
    return -1;
  }

  if (parse_port(argv[1], port_number) != 0) {
    log_fatal("Usage: %s <port_number> <project_dir>");
    return -1;
  }

  signal_init();

  (void)string_init(root_dir, argv[2]);
  if (get_path_type(root_dir) != PATH_DIR) {
    log_fatal("Usage: %s <port_number> <project_dir>");
    return -1;
  }

  return 0;
}

int main(int argc, char *argv[]) {
  uint16_t port_number;
  string_t root_dir;
  setup(argc, argv, &port_number, &root_dir);

  int sockfd = open_socket(port_number);
  job_queue_t queue;
  queue_init(&queue);

  thread_pool_t pool;
  if (thread_pool_init(&pool, &queue, &root_dir) != 0) {
    return EXIT_FAILURE;
  }

  log_info("Server accepting connections...");
  while (server_running) {
    log_trace("Socket_fd: %d", sockfd);
    int client = get_client(sockfd);
    if (client >= 0) {
      queue_push(&queue, client);
    }
  }

  log_info("Stopping server...");

  queue_shutdown(&queue);
  thread_pool_wait(&pool);
  queue_destroy(&queue);
  string_destroy(&root_dir);
  close(sockfd);

  return EXIT_SUCCESS;
}
