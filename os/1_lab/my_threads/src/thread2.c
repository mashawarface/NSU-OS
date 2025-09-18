#define _GNU_SOURCE
#include <pthread.h>
#include <stdio.h>

void *my_thread(void *arg) {
//   return (void *)42;
  return "Hello World";
}

int main() {
  pthread_t tid;
//   int val;
  char *val;

  pthread_create(&tid, NULL, my_thread, NULL);

  pthread_join(tid, (void **)&val);

//   printf("%d\n", (int)val);
  printf("%s\n", (char *)val);

  return 0;
}
