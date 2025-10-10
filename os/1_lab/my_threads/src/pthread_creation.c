#define _GNU_SOURCE

#include <pthread.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

int global = 1;

void *my_thread(void *arg) {
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

  sleep(1000);

  return NULL;
}

int main() {
  pthread_t tid1, tid2, tid3, tid4, tid5;

  int err1 = pthread_create(&tid1, NULL, my_thread, NULL);
  int err2 = pthread_create(&tid2, NULL, my_thread, NULL);
  int err3 = pthread_create(&tid3, NULL, my_thread, NULL);
  int err4 = pthread_create(&tid4, NULL, my_thread, NULL);
  int err5 = pthread_create(&tid5, NULL, my_thread, NULL);

  if (err1 || err2 || err3 || err4 || err5) {
    printf("error in creating thread!\n");
    return -1;
  }


  pthread_join(tid1, NULL);
  pthread_join(tid2, NULL);
  pthread_join(tid3, NULL);
  pthread_join(tid4, NULL);
  pthread_join(tid5, NULL);

  printf(
      "---\nresults of pthread_create(): tid1 = %ld ; tid2 = %ld; tid3 = %ld; "
      "tid4 = %ld; tid5 = %ld\n---\n",
      tid1, tid2, tid3, tid4, tid5);

  return 0;
}
