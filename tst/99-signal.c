#include <stdio.h>
#include <assert.h>
#include "../src/thread.h"

#define THREAD_SIG_TERM 0   // Terminate thread signal
#define THREAD_SIG_USR1 1   // User-defined signal

// A flag to confirm the signal handler was invoked
static int signal_handled = 0;

// Signal handler
static void handle_usr1(int sig) {
  printf("Received signal: %d\n", sig);
  signal_handled = 1;
}

// Thread function: registers handler and waits
static void *signal_thread(void *arg __attribute__((unused))) {
  thread_signal(thread_self(), THREAD_SIG_USR1, handle_usr1);

  // Yield to let the main thread send the signal
  thread_yield();

  // Deliver any pending signals
  deliver_pending_signals((struct my_thread *)thread_self());

  return (void*)(intptr_t)signal_handled;
}

int main() {
  thread_t th;
  void *res = NULL;
  int err;

  err = thread_create(&th, signal_thread, NULL);
  assert(!err);

  // Give the thread time to set up its signal handler
  thread_yield();

  // Send the signal
  thread_kill(th, THREAD_SIG_USR1);

  // Join and check result
  err = thread_join(th, &res);
  assert(!err);
  assert(res == (void*)1);

  printf("signal test OK\n");
  return 0;
}
