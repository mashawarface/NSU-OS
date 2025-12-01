#include <assert.h>
#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "../lib/include/list_mutex.h"

#define THREAD_COUNT 6
#define THREAD_SWAP_COUNT 3
#define THREAD_SEARCH_COUNT 3

#define LIST_SIZE 5

void *search_ascend(void *arg) {
  thread_arg_t *thread_arg = (thread_arg_t *)arg;

  list_t *list = thread_arg->list;
  size_t *iterations = thread_arg->counter, counter = 0;

  node_t *current = list->first;

  int prev_lenght = 0, curr_length;

  while (1) {
    pthread_mutex_lock(&current->sync);

    curr_length = strlen(current->buf);
    curr_length > prev_lenght ? counter++ : (counter = 0);
    prev_lenght = curr_length;

    pthread_mutex_unlock(&current->sync);

    current = current->next;

    if (current == NULL) {
      (*iterations)++;

      current = list->first;
    }
  }

  return NULL;
}

void *search_descend(void *arg) {
  thread_arg_t *thread_arg = (thread_arg_t *)arg;

  list_t *list = thread_arg->list;
  size_t *iterations = thread_arg->counter, counter = 0;

  node_t *current = list->first;

  int prev_lenght = 0, curr_length;

  while (1) {
    pthread_mutex_lock(&current->sync);

    curr_length = strlen(current->buf);
    curr_length < prev_lenght ? counter++ : (counter = 0);
    prev_lenght = curr_length;

    pthread_mutex_unlock(&current->sync);

    current = current->next;

    if (current == NULL) {
      (*iterations)++;

      current = list->first;
    }
  }

  return NULL;
}

void *search_equal(void *arg) {
  thread_arg_t *thread_arg = (thread_arg_t *)arg;

  list_t *list = thread_arg->list;
  size_t *iterations = thread_arg->counter, counter = 0;

  node_t *current = list->first;

  int prev_lenght = 0, curr_length;

  while (1) {
    pthread_mutex_lock(&current->sync);

    curr_length = strlen(current->buf);
    curr_length == prev_lenght ? counter++ : (counter = 0);
    prev_lenght = curr_length;

    pthread_mutex_unlock(&current->sync);

    current = current->next;

    if (current == NULL) {
      (*iterations)++;
      current = list->first;
    }
  }

  return NULL;
}

void *swap(void *arg) {
  thread_arg_t *thread_arg = (thread_arg_t *)arg;

  list_t *list = thread_arg->list;
  size_t *counter = thread_arg->counter;

  int err;

  if (!list) {
    return NULL;
  }

  while (1) {
    node_t *prev = NULL, *curr = NULL, *next = NULL;

    while (1) {
      int make_swap = rand() % 2;

      if (!make_swap) {
        continue;
      }

      if (!prev) {
        err = pthread_mutex_lock(&list->sync);
        if (err) {
          printf("Can't lock mutex bc of %s!\n", strerror(err));
          break;
        }

        node_t *first = list->first;
        if (!first || !first->next) {
          break;
        }

        err = pthread_mutex_lock(&first->sync);
        if (err) {
          pthread_mutex_unlock(&list->sync);
          printf("Can't lock mutex bc of %s!\n", strerror(err));
          break;
        }

        curr = first;
        if (!curr->next) {
          pthread_mutex_unlock(&list->sync);
          pthread_mutex_unlock(&curr->sync);
          break;
        }

        next = curr->next;
        err = pthread_mutex_lock(&next->sync);
        if (err) {
          pthread_mutex_unlock(&list->sync);
          pthread_mutex_unlock(&curr->sync);
          printf("Can't lock mutex bc of %s!\n", strerror(err));

          break;
        }

        if (curr->next != next) {
          pthread_mutex_unlock(&list->sync);
          pthread_mutex_unlock(&curr->sync);
          pthread_mutex_unlock(&next->sync);

          break;
        }

        list->first = next;
        curr->next = next->next;
        next->next = curr;

        (*counter)++;

        prev = curr;

        pthread_mutex_unlock(&list->sync);
        pthread_mutex_unlock(&curr->sync);
        pthread_mutex_unlock(&next->sync);
      } else {
        err = pthread_mutex_lock(&prev->sync);
        if (err) {
          printf("Can't lock mutex bc of %s!\n", strerror(err));
          break;
        }

        if (!prev->next || !prev->next->next) {
          pthread_mutex_unlock(&prev->sync);
          prev = NULL;
          break;
        }

        curr = prev->next;
        err = pthread_mutex_lock(&curr->sync);
        if (err) {
          pthread_mutex_unlock(&prev->sync);
          printf("Can't lock mutex bc of %s!\n", strerror(err));
          break;
        }

        next = curr->next;
        err = pthread_mutex_lock(&next->sync);
        if (err) {
          pthread_mutex_unlock(&curr->sync);
          pthread_mutex_unlock(&prev->sync);
          printf("Can't lock mutex bc of %s!\n", strerror(err));
          break;
        }

        if (prev->next != curr || curr->next != next) {
          pthread_mutex_unlock(&next->sync);
          pthread_mutex_unlock(&curr->sync);
          pthread_mutex_unlock(&prev->sync);

          break;
        }

        prev->next = next;
        curr->next = next->next;
        next->next = curr;

        (*counter)++;

        node_t *tmp = prev;
        prev = curr;

        pthread_mutex_unlock(&tmp->sync);
        pthread_mutex_unlock(&curr->sync);
        pthread_mutex_unlock(&next->sync);
      }
    }
  }
}

int main() {
  // srand(time(0));

  list_t *list = list_init(LIST_SIZE);

  int err;

  pthread_t tids[THREAD_COUNT];

  void *(*thread_funcs[THREAD_COUNT])(void *) = {
      search_ascend, search_descend, search_equal, swap, swap, swap};

  size_t counters[THREAD_COUNT] = {0, 0, 0, 0, 0, 0};

  thread_arg_t args[THREAD_COUNT] = {
      {list, &counters[0]}, {list, &counters[1]}, {list, &counters[2]},
      {list, &counters[3]}, {list, &counters[4]}, {list, &counters[5]},
  };

  for (int i = 0; i < THREAD_COUNT; i++) {
    err = pthread_create(&tids[i], NULL, thread_funcs[i], &args[i]);
    if (err != 0) {
      printf("Error in creating thread #%d because of %s!\n", i, strerror(err));
      return 1;
    }
  }

  while (1) {
    printf("List stats:\n"
           "\tThread (ascending pairs):  %zu iteration(s)\n"
           "\tThread (descending pairs): %zu iteration(s)\n"
           "\tThread (equal pairs):      %zu iteration(s)\n"
           "\tFisrt swapping thread:     %zu successful swap(s)\n"
           "\tSecond swapping thread:    %zu successful swap(s)\n"
           "\tThird swapping thread:     %zu successful swap(s)\n",
           counters[0], counters[1], counters[2], counters[3], counters[4],
           counters[5]);
    sleep(1);
  }

  for (int i = 0; i < THREAD_COUNT; i++) {
    err = pthread_join(tids[i], NULL);
    if (err != 0) {
      printf("Can`t join thread #%d because of %s!\n", i, strerror(err));
      return 1;
    }
  }

  return 0;
}
