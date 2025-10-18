#include <bits/types/sigset_t.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

void signal_handler(int sig) {
  char *message = "\nReceived signal!\n";
  write(1, message, strlen(message));
}

void *thread_ignore_all(void *arg) {
  (void)arg;

  while (1) {
    printf("Working: pid = %d\n", getpid());
    sleep(3);
  }

  return NULL;
}

void *thread_sigint(void *arg) {
  (void)arg;

  signal(SIGINT, signal_handler);

  while (1) {
    printf("Working: pid = %d\n", getpid());
    sleep(3);
  }

  return NULL;
}

void *thread_sigquit(void *arg) {
  (void)arg;

  while (1) {
    printf("Working: pid = %d\n", getpid());
    sleep(3);
  }

  return NULL;
}

#define THREAD_COUNT 3

int main() {
  pthread_t tids[THREAD_COUNT];
  int err;

struct sigaction act;
act.sa_flags = 

  err = pthread_create(&tids[0], NULL, thread_ignore_all, NULL);
  if (err != 0) {
    printf("Error in creating thread #1 because of %s!\n", strerror(err));
    return 1;
  }

  err = pthread_create(&tids[1], NULL, thread_sigint, NULL);
  if (err != 0) {
    printf("Error in creating thread #2 because of %s!\n", strerror(err));
    return 1;
  }

  err = pthread_create(&tids[2], NULL, thread_sigquit, NULL);
  if (err != 0) {
    printf("Error in creating thread #3 because of %s!\n", strerror(err));
    return 1;
  }

  for (int i = 0; i < THREAD_COUNT; i++) {
    pthread_join(tids[i], NULL);
    if (err != 0) {
      printf("Can`t join thread #%d because of %s!\n", i, strerror(err));
      return 1;
    }
  }

  return 0;
}
