#include "sig.h"

volatile sig_atomic_t server_running = 1;

static void handle_signal(int signo) {
  (void)signo;
  server_running = 0;
}

void signal_init(void) {
  struct sigaction sa = {};

  sa.sa_handler = handle_signal;

  sigaction(SIGINT, &sa, nullptr);
  sigaction(SIGTERM, &sa, nullptr);
}
