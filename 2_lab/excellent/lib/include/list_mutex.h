#ifndef QUEUE_H
#define QUEUE_H

#include <pthread.h>
#include <time.h>

#define MAX_LENGTH 100

typedef struct node {
  char buf[MAX_LENGTH];
  struct node *next;
  pthread_mutex_t sync;
} node_t;

typedef struct list {
  node_t *first;
  pthread_mutex_t sync;
} list_t;

list_t *list_init(size_t size);
void print_list(list_t *list);


#endif // QUEUE_H
