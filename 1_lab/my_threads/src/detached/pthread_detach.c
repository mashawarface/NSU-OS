#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

void *my_thread(void *arg) {
  (void)arg;

  int err = pthread_detach(pthread_self());
  if (err != 0) {
    printf("Error in detaching thread because of %s!\n", strerror(err));
    return NULL;
  }

  printf("self: %ld\n", pthread_self());

  return NULL;
}

int main() {
  pthread_t tid;
  int err;
  long counter = 0;

  while (1) {
    err = pthread_create(&tid, NULL, my_thread, NULL);

    if (err != 0) {
      printf("STOP WORKING BC OF %s\n", strerror(err));
    }

    if (err == 0) {
      counter++;
      printf("%ld\n", counter);
    }
  }

  return 0;
}
