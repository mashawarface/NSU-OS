#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>

#include "../lib/include/mythread.h"

void *my_thread(void *arg, mythread_t thread) {
  (void)arg;

  return (void *)42;
}

int main() {
  mythread_t tid;
  int err;
  int val;

  err = mythread_create(&tid, my_thread, NULL);
  if (err != 0) {
    printf("Error in creating thread because of %s!\n", strerror(err));
    return -1;
  }

  err = mythread_join(tid, (void **)&val);
  if (err != 0) {
    printf("Can`t join thread because of %s!\n", strerror(err));
    return -1;
  }

  printf("%d\n", val);

  return 0;
}