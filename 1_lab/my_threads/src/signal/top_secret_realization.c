#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define THREAD_COUNT 3

void *thread_ignore_all(void *arg) {
  (void)arg;

  return NULL;
}

void *thread_sigint(void *arg) {
  (void)arg;

  return NULL;
}

void *thread_sigquit(void *arg) {
  (void)arg;

  return NULL;
}

int main() {
  pthread_t tids[THREAD_COUNT];
  void *(*thread_funcs[THREAD_COUNT])(void *) = {thread_ignore_all,
                                                 thread_sigint, thread_sigquit};
  int err;

  for (int i = 0; i < THREAD_COUNT; i++) {
    err = pthread_create(&tids[i], NULL, thread_funcs[i], NULL);
    if (err != 0) {
      printf("Error in creating thread #%d because of %s!\n", i, strerror(err));
      return 1;
    }
  }

  for (int i = 0; i < THREAD_COUNT; i++) {
    err = pthread_join(tids[i], NULL);
    if (err != 0) {
      printf("Can`t join thread #%d because of %s!\n", i, strerror(err));
      return 1;
    }
  }
}

