#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void cleanup_handler(void *arg) {
  if (arg != NULL) {
    free(arg);
    arg = NULL;
    printf("HANDLER HAS EXECUTED!\n");
  }
}

void *my_thread(void *arg) {
  (void)arg;

  char *string = malloc(sizeof("hello world"));
  if (string == NULL) {
    printf("Error in memallocation!\n");
    return NULL;
  }

  strcpy(string, "hello world");

  pthread_cleanup_push(cleanup_handler, string);

  while (1) {
    printf("%s\n", string);
  }

  /*When a thread is canceled, all of the stacked clean-up handlers are popped
     and executed in the reverse of the order in which they were pushed onto the
     stack.*/
  pthread_cleanup_pop(0);

  return NULL;
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

  return 0;
}
