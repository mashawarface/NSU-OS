#define _GNU_SOURCE

#include <string.h>
#include <pthread.h>
#include <stdio.h>

void *my_thread(void *arg) {
  (void)arg;

  return (void *)42;
  //  return "Hello World";
}

int main() {
  pthread_t tid;
  int val;
  // char *val;

  int err = pthread_create(&tid, NULL, my_thread, NULL);

  if (err != 0) {
    printf("Error in creating thread because of %s!\n", strerror(err));
    return -1;
  }

  err = pthread_join(tid, (void **)&val);

  if (err != 0) {
    printf("Can`t join thread because of %s!\n", strerror(err));
    return -1;
  }

  printf("%d\n", val);
  // printf("%s\n", (char *)val);

  return 0;
}
