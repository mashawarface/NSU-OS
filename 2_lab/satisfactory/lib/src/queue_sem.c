#define _GNU_SOURCE

#include <assert.h>
#include <bits/pthreadtypes.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// #include "../include/queue.h"

typedef struct _QueueNode {
  int val;
  struct _QueueNode *next;
} qnode_t;

typedef struct _Queue {
  qnode_t *first;
  qnode_t *last;

  pthread_t qmonitor_tid;

  sem_t add_available;
  sem_t get_available;
  sem_t sem;

  int count;
  int max_count;

  // queue statistics
  long add_attempts;
  long get_attempts;
  long add_count;
  long get_count;
} queue_t;

queue_t *queue_init(int max_count);
void queue_destroy(queue_t *q);
int queue_add(queue_t *q, int val);
int queue_get(queue_t *q, int *val);
void queue_print_stats(queue_t *q);

void *qmonitor(void *arg) {
  queue_t *q = (queue_t *)arg;

  printf("qmonitor: [%d %d %d]\n", getpid(), getppid(), gettid());

  while (1) {
    queue_print_stats(q);
    sleep(1);
  }

  return NULL;
}

queue_t *queue_init(int max_count) {
  int err;

  queue_t *q = malloc(sizeof(queue_t));
  if (!q) {
    printf("Cannot allocate memory for a queue\n");
    abort();
  }

  q->first = NULL;
  q->last = NULL;
  q->max_count = max_count;
  q->count = 0;

  q->add_attempts = q->get_attempts = 0;
  q->add_count = q->get_count = 0;

  err = sem_init(&q->add_available, 0, q->max_count);
  if (err != 0) {
    printf("Error in initializing of spin because of %s!\n", strerror(err));
    abort();
  }

  err = sem_init(&q->get_available, 0, 0);
  if (err != 0) {
    printf("Error in initializing of spin because of %s!\n", strerror(err));
    abort();
  }

  err = sem_init(&q->sem, 0, 1);
  if (err != 0) {
    printf("Error in initializing of spin because of %s!\n", strerror(err));
    abort();
  }

  err = pthread_create(&q->qmonitor_tid, NULL, qmonitor, q);
  if (err) {
    printf("queue_init: pthread_create() failed: %s\n", strerror(err));
    abort();
  }

  return q;
}

void queue_destroy(queue_t *q) {
  while (q->first) {
    qnode_t *tmp = q->first;
    q->first = q->first->next;
    free(tmp);
  }

  sem_destroy(&q->add_available);
  sem_destroy(&q->get_available);
  sem_destroy(&q->sem);

  free(q);
}

int queue_add(queue_t *q, int val) {
  sem_wait(&q->add_available);

  sem_wait(&q->sem);

  q->add_attempts++;

  assert(q->count <= q->max_count);

  qnode_t *new = malloc(sizeof(qnode_t));
  if (!new) {
    printf("Cannot allocate memory for new node\n");
    abort();
  }

  new->val = val;
  new->next = NULL;

  if (!q->first) {
    q->first = q->last = new;
  } else {
    q->last->next = new;
    q->last = q->last->next;
  }

  q->count++;

  q->add_count++;

  sem_post(&q->sem);

  sem_post(&q->get_available);

  return 1;
}

int queue_get(queue_t *q, int *val) {
  sem_wait(&q->get_available);

  sem_wait(&q->sem);

  q->get_attempts++;

  assert(q->count >= 0);

  qnode_t *tmp = q->first;

  *val = tmp->val;
  q->first = q->first->next;

  free(tmp);
  q->count--;

  q->get_count++;

  sem_post(&q->sem);

  sem_post(&q->add_available);

  return 1;
}

void queue_print_stats(queue_t *q) {
  printf("queue stats: current size %d; attempts: (%ld %ld %ld); counts (%ld "
         "%ld %ld)\n",
         q->count, q->add_attempts, q->get_attempts,
         q->add_attempts - q->get_attempts, q->add_count, q->get_count,
         q->add_count - q->get_count);
}
