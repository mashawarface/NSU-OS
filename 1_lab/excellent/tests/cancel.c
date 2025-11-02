#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "../lib/include/mythread.h"

int counter = 0;

void *my_thread(void *arg, mythread_t thread) {
  while (1) {
    counter += 1;
    mythread_testcancel(thread);
  }
}

int main() {
  mythread_t tid;
  int err;
  void *res;

  err = mythread_create(&tid, my_thread, tid);
  if (err != 0) {
    printf("Error in creating thread because of %s!\n", strerror(err));
    return 1;
  }

  sleep(2);

  err = mythread_cancel(tid);
  if (err != 0) {
    printf("Error in canceling thread because of %s!\n", strerror(err));
    return 1;
  }

  err = mythread_join(tid, &res);
  if (err != 0) {
    printf("Error in joining thread because of %s!\n", strerror(err));
    return 1;
  }

  if (res == THREAD_CANCELED) {
    printf("Thread was canceled!\n");
  } else {
    printf("Thread wasn`t canceled!\n");
  }

  printf("%d\n", counter);

  return 0;
}