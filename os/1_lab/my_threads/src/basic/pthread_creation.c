#define _GNU_SOURCE
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

int global = 1;

void *my_thread(void *arg) {
  (void)arg;

  int local = 1;
  static int static_local = 1;
  const int const_local = 1;

  // changing local and global vars
  local += 1;
  global += 1;

  printf("my thread getpid(): %d; getppid(): %d; gettid(): %d; pthread_self(): "
         "%ld; addresses of: local = %p; static local = %p; const local = %p; "
         "global = %p\nchanged var: local = %d; global = %d\n",
         getpid(), getppid(), gettid(), pthread_self(), &local, &static_local,
         &const_local, &global, local, global);

  printf("%p", (void *)pthread_self());

  return NULL;
}

int main() {
  pthread_t tids[5];
  int err;

  for (int i = 0; i < 5; i++) {
    err = pthread_create(&tids[i], NULL, my_thread, NULL);
    if (err != 0) {
      printf("error in creating thread #%d because of %s!\n", i, strerror(err));
      return -1;
    }
  }

  for (int i = 0; i < 5; i++) {
    err = pthread_join(tids[i], NULL);
    if (err != 0) {
      printf("Can`t join thread #%d because of %s!\n", i, strerror(err));
      return -1;
    }
  }

  printf(
      "---\nresults of pthread_create(): tid1 = %ld ; tid2 = %ld; tid3 = %ld; "
      "tid4 = %ld; tid5 = %ld\n---\n",
      tids[0], tids[1], tids[2], tids[3], tids[4]);

  return 0;
}
