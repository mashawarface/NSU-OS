#include <assert.h>
#include <pthread.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "../lib/include/list_mutex.h"

size_t counter_ascending_thread = 0;

void *search_ascendind(void *arg) {
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
      counter_ascending_thread++;
      current = list->first;
    }
  }

  return NULL;
}

size_t counter_descending_thread = 0;

void *search_descending(void *arg) {
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
      counter_descending_thread++;
      current = list->first;
    }
  }

  return NULL;
}

size_t counter_equal_thread = 0;

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
      counter_equal_thread++;
      current = list->first;
    }
  }

  return NULL;
}
// TODO: add global var to args
/* Swap current node with next node. */
void *swap(void *arg) {
  list_t *list = (list_t *)arg;
  node_t *current = list->first;
  node_t *prev = current;
  node_t *next = current->next;

  int make_swap = 0;

  while (1) {
    make_swap = rand() % 2;

    if (make_swap) {
      if (current == list->first) {
        pthread_mutex_lock(&list->sync);
        pthread_mutex_lock(&current->sync);
        pthread_mutex_lock(&next->sync);

        node_t *tmp = next->next;

        next->next = current;
        current->next = tmp;
        list->first = next;

        free(tmp);

        pthread_mutex_unlock(&next->sync);
        pthread_mutex_unlock(&current->sync);
        pthread_mutex_unlock(&list->sync);
      }

      else {
        pthread_mutex_lock(&prev->sync);
        pthread_mutex_lock(&current->sync);
        pthread_mutex_lock(&next->sync);

        node_t *tmp = next->next;

        prev->next = next;
        next->next = current;
        current->next = tmp;

        free(tmp);

        pthread_mutex_unlock(&next->sync);
        pthread_mutex_unlock(&current->sync);
        pthread_mutex_unlock(&prev->sync);
      }
    }

    /* If we on the last element, then we do not swap, just make current node
   point to the start of the list. */
    if (!current->next) {
      prev = current = list->first;
      next = current->next;
    } else {
      prev = current;
      current = current->next;
      next = current->next;
    }
  }
}

int main() {
  list_t *list = list_init(10);

  print_list(list);

  return 0;
}
