#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int counter = 0;

void *my_thread(void *arg) {
  (void)arg;
  int err;

  err = pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

  err = pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
  if (err != 0) {
    printf("Error in setting cancel type thread because of %s!\n",
           strerror(err));
    return NULL;
  }

  while (1) {
    counter += 1;
    // pthread_testcancel();
  }
}

int main() {
  pthread_t tid;
  int err;
  void *res;

  err = pthread_create(&tid, NULL, my_thread, NULL);
  if (err != 0) {
    printf("Error in creating thread because of %s!\n", strerror(err));
    return 1;
  }

  err = pthread_cancel(tid);
  if (err != 0) {
    printf("Error in canceling thread because of %s!\n", strerror(err));
    return 1;
  }

  err = pthread_join(tid, &res);
  if (err != 0) {
    printf("Error in joining thread because of %s!\n", strerror(err));
    return 1;
  }

  if (res == PTHREAD_CANCELED) {
    printf("Thread was canceled!\n");
  } else {
    printf("Thread wasn`t canceled!\n");
  }

  printf("%d", counter);

  return 0;
}
