#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "../lib/include/mythread.h"

int global = 1;

void *my_thread(void *arg, mythread_t thread) {
  (void)arg;

  int local = 1;
  static int static_local = 1;
  const int const_local = 1;

  local += 1;
  global += 1;

  printf("my thread getpid(): %d; getppid(): %d; gettid %d; mythread_self(): "
         "%lu; addresses of: "
         "local = %p; static local = %p; const local = %p; "
         "global = %p\nchanged var: local = %d; global = %d\n",
         getpid(), getppid(), gettid(), mythread_self(thread), &local,
         &static_local, &const_local, &global, local, global);

  return NULL;
}

int main() {
  mythread_t tids[5];
  int err;

  for (int i = 0; i < 5; i++) {
    err = mythread_create(&tids[i], my_thread, NULL);
    if (err != 0) {
      printf("error in creating thread #%d because of %s!\n", i, strerror(err));
      return -1;
    }
  }

  for (int i = 0; i < 5; i++) {
    err = mythread_join(tids[i], NULL);
    if (err != 0) {
      printf("Can`t join thread #%d because of %s!\n", i, strerror(err));
      return -1;
    }
  }
}