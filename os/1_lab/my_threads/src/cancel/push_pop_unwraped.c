#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void cleanup_handler(void *arg) {
  if (arg != NULL) {
    free(arg);
    arg = NULL;
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

  //   pthread_cleanup_push(cleanup_handler, string);

  do {
    __pthread_unwind_buf_t __cancel_buf;                                                    // create buffer
    void (*__cancel_routine)(void *) = (cleanup_handler);                                   // parse arguments
    void *__cancel_arg = (string);
    int __not_first_call = __sigsetjmp(
        (struct __jmp_buf_tag *)(void *)(__cancel_buf.__cancel_jmp_buf), (0));
    if (__builtin_expect((__not_first_call), 0)) {
      __cancel_routine(__cancel_arg);
      __pthread_unwind_next(&__cancel_buf);
    }
    __pthread_register_cancel(&__cancel_buf);
    do {

      while (1) {
        printf("%s\n", string);
      }

      do {
      } while (0);
    } while (0);
    __pthread_unregister_cancel(&__cancel_buf);
    if (0)
      __cancel_routine(__cancel_arg);
  } while (0);

  //   pthread_cleanup_pop(0);

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
