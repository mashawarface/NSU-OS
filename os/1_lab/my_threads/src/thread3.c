#define _GNU_SOURCE
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

void *my_thread(void *arg) {
  printf("pthread_self(): %ld\n", pthread_self());

  // pthread_detach(pthread_self());

  return NULL;
}

int main() {
  pthread_t tid;
  pthread_attr_t attr;
  long counter = 0;

  // pthread_attr_init(&attr);
  // pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

  while (1) {
    pthread_create(&tid, &attr, my_thread, NULL);
    printf("%ld", ++counter);
  }

  // pthread_attr_destroy(&attr);

  return 0;
}
