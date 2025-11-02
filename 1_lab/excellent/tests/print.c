#include "/home/mary/NSU-OS/1_lab/excellent/lib/include/mythread.h"
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

void *my_routine(void *args, mythread_t thread) {
  (void)args;

  printf("Hello!\n");

  return NULL;
}

int main(void) {
  mythread_t tid1, tid2, tid3;
  int err;

  err = mythread_create(&tid1, my_routine, NULL);
  if (err != 0) {
    printf("Error in creating a thread, because of %s!\n", strerror(err));
    return 1;
  }

  err = mythread_create(&tid2, my_routine, NULL);
  if (err != 0) {
    printf("Error in creating a thread, because of %s!\n", strerror(err));
    return 1;
  }

  err = mythread_join(tid1, NULL);
  if (err != 0) {
    printf("Error in joining a thread, because of %s!\n", strerror(err));
    return 1;
  }

  err = mythread_join(tid2, NULL);
  if (err != 0) {
    printf("Error in joining a thread, because of %s!\n", strerror(err));
    return 1;
  }

  return 0;
}
