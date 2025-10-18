#include <pthread.h>
#include <stdio.h>
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

  return NULL;
}

int main() {
  pthread_t tid;
  int err;

  my_struct m;

  m.a = 1;
  m.b = "a";

  err = pthread_create(&tid, NULL, my_thread, &m);
  if (err != 0) {
    printf("Error in creating thread because of %s!\n", strerror(err));
    return -1;
  }

  err = pthread_join(tid, NULL);
  if (err != 0) {
    printf("Can`t join thread because of %s\n", strerror(err));
    return -1;
  }

  return 0;
}
