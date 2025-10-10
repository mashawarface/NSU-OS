#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// typedef struct {
//   int a;
//   char *b;
// } my_struct;

// void *my_thread(void *arg) {
//   my_struct *m = (my_struct *)arg;

//   printf("%d %s\n", m->a, m->b);

//   return NULL;
// }

// int main() {
//   pthread_t tid;

//   my_struct m;

//   m.a = 1;
//   m.b = "a";

//   int err = pthread_create(&tid, NULL, my_thread, &m);

//   if (err != 0) {
//     return 1;
//   }
//   pthread_join(tid, NULL);

//   return 0;
// }

typedef struct {
  int a;
  char *b;
} my_struct;

void *my_thread(void *arg) {
  pthread_detach(pthread_self());

  my_struct *m = (my_struct *)arg;

  printf("%d %s\n", m->a, m->b);

  return NULL;
}

int main() {
  pthread_t tid;


  my_struct *m = malloc(sizeof(my_struct));

  m->a = 1;

  m->b = malloc(sizeof(char));
  m->b = "a";

  int err = pthread_create(&tid, NULL, my_thread, m);

  if (err != 0) {
    return 1;
  }
  sleep(1);

  return 0;
}
