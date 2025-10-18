#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct {
  int a;
  char *b;
} my_struct;

void *my_thread(void *arg) {
  (void)arg;

  int err = pthread_detach(pthread_self());
  if (err != 0) {
    printf("Error in detaching thread because of %s!\n", strerror(err));
    return NULL;
  }

  my_struct *m = (my_struct *)arg;

  printf("%d %s\n", m->a, m->b);

  free(m);

  return NULL;
}

int main() {
  pthread_t tid;
  int err;

  my_struct *m = malloc(sizeof(my_struct));

  if (m == NULL) {
    printf("Error in memallocation!\n");
    return -1;
  }

  m->a = 1;
  m->b = "a";

  err = pthread_create(&tid, NULL, my_thread, m);
  if (err != 0) {
    printf("Error in creating thread because of %s!\n", strerror(err));
    return -1;
  }

  pthread_exit(NULL);

  return 0;
}
