#ifndef SIG_H
#define SIG_H

#include <signal.h>

// Global flag: 1 = Running, 0 = Stop
extern volatile sig_atomic_t server_running;

void signal_init(void);

#endif // !SIG_H
