#include "/home/mary/NSU-OS/1_lab/excellent/lib/include/mythread.h"
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

void *my_routine(void *args) {
  (void)args;

  char *msg = "hello";

  for (size_t i = 0; i < 2; i++) {
    puts(msg);
    // sleep(1);
  }

  return NULL;
}

int main(void) {
  // printf("%d", getpid());

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

  // mythread_detach(tid1);
  // mythread_detach(tid2);

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

  // sleep(1000);

  return 0;
}
