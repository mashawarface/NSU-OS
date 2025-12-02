#ifndef QUEUE_H
#define QUEUE_H

#include <pthread.h>
#include <time.h>

#define MAX_LENGTH 10

typedef struct node {
  char buf[MAX_LENGTH];
  struct node *next;
  pthread_spinlock_t sync;
} node_t;

typedef struct list {
  node_t *first;
  pthread_spinlock_t sync;
} list_t;

typedef struct search_args {
  list_t *list;
  size_t *counter;
  int (*compare)(int curr, int prev);
} search_thread_arg_t;

typedef struct swap_args {
  list_t *list;
  size_t *counter;
} swap_thread_arg_t;

list_t *list_init(size_t size);
void list_destroy(list_t *list);
void print_list(list_t *list);


#endif // QUEUE_H
