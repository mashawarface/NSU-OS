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

size_t counter_ascend = 0, counter_descend = 0, counter_equal = 0,
       counter_swap_first = 0, counter_swap_second = 0, counter_swap_third = 0;

void *search_ascend(void *arg) {
  list_t *list = (list_t *)arg;
  node_t *current = list->first;

  size_t counter;
  int prev_lenght = 0, curr_length;

  while (1) {
    pthread_mutex_lock(&current->sync);

    curr_length = strlen(current->buf);
    curr_length > prev_lenght ? counter++ : (counter = 0);
    prev_lenght = curr_length;

    pthread_mutex_unlock(&current->sync);

    current = current->next;

    if (current == NULL) {
      counter_ascend++;
      current = list->first;
    }
  }

  return NULL;
}

void *search_descend(void *arg) {
  list_t *list = (list_t *)arg;
  node_t *current = list->first;

  size_t counter;
  int prev_lenght = 0, curr_length;

  while (1) {
    pthread_mutex_lock(&current->sync);

    curr_length = strlen(current->buf);
    curr_length < prev_lenght ? counter++ : (counter = 0);
    prev_lenght = curr_length;

    pthread_mutex_unlock(&current->sync);

    current = current->next;

    if (current == NULL) {
      counter_descend++;
      current = list->first;
    }
  }

  return NULL;
}

void *search_equal(void *arg) {
  list_t *list = (list_t *)arg;
  node_t *current = list->first;

  size_t counter;
  int prev_lenght = 0, curr_length;

  while (1) {
    pthread_mutex_lock(&current->sync);

    curr_length = strlen(current->buf);
    curr_length == prev_lenght ? counter++ : (counter = 0);
    prev_lenght = curr_length;

    pthread_mutex_unlock(&current->sync);

    current = current->next;

    if (current == NULL) {
      counter_equal++;
      current = list->first;
    }
  }

  return NULL;
}

/* Swap current node with next node. */
void *swap(void *arg) {
  list_t *list = (list_t *)arg;

  int make_swap = 1;

  while (1) {
    node_t *prev = NULL, *next = NULL;

    node_t *current = list->first;

    while (current && current->next) {
      // make_swap = rand() % 2;

      if (make_swap) {
        puts("list before swap!");
        print_list(list);

        next = current->next;

        current->next = next->next;
        next->next = current;

        if (!prev)
          list->first = next;
        else
          prev->next = next;

        prev = next;

        counter_swap_third++;

        puts("list after swap!");
        print_list(list);
        sleep(1);
      } else {
        puts("do not swap!");
        prev = current;
        current = current->next;
      }
    }
  }
}

// typedef struct args {
//   list_t *list;
//   size_t counter;
// } thread_arg_t;

int main() {
  srand(time(0));

  list_t *list = list_init(5);

  pthread_t tids[THREAD_COUNT];
  void *(*thread_funcs[THREAD_COUNT])(void *) = {
      search_ascend, search_descend, search_equal, swap, swap, swap};
  int err;

  // for (int i = 0; i < THREAD_COUNT; i++) {
  //   err = pthread_create(&tids[i], NULL, thread_funcs[i], list);
  //   if (err != 0) {
  //     printf("Error in creating thread #%d because of %s!\n", i,
  //     strerror(err)); return 1;
  //   }
  // }

  // pthread_create(&tids[0], NULL, search_ascend, list);
  // pthread_create(&tids[1], NULL, search_descend, list);
  // pthread_create(&tids[2], NULL, search_equal, list);
  pthread_create(&tids[3], NULL, swap, list);

  while (1) {
    printf("List stats:\n"
           "\tThread (ascending pairs):  %zu iteration(s)\n"
           "\tThread (descending pairs): %zu iteration(s)\n"
           "\tThread (equal pairs):      %zu iteration(s)\n"
           "\tFisrt swapping thread:     %zu successful swap(s)\n"
           "\tSecond swapping thread:    %zu successful swap(s)\n"
           "\tThird swapping thread:     %zu successful swap(s)\n",
           counter_ascend, counter_descend, counter_equal, counter_swap_first,
           counter_swap_second, counter_swap_third);
    sleep(2);
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
