#include <assert.h>
#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/list_mutex.h"

void generate_string(char *buf, int max_length) {
  int length = rand() % (max_length - 1) + 1;

  for (int i = 0; i < length; i++) {
    buf[i] = '1';
  }

  buf[length] = '\0';
}

list_t *list_init(size_t size) {
  int err;

  list_t *list = malloc(sizeof(list_t));
  if (list == NULL) {
    printf("Error in allocating memory for list!\n");
    return NULL;
  }

  list->first = NULL;

  err = pthread_mutex_init(&list->sync, NULL);
  if (err != 0) {
    printf("Error in initializing of mutex because of %s!\n", strerror(err));
    free(list);
    return NULL;
  }

  node_t *current;

  size_t added = 0;

  while (added != size) {
    /* Start of node's initializing */
    node_t *node = malloc(sizeof(node_t));
    if (node == NULL) {
      printf("Error in allocating memory for node!\n");
      list_destroy(list);
      return NULL;
    }

    generate_string(node->buf, MAX_LENGTH);

    err = pthread_mutex_init(&node->sync, NULL);
    if (err != 0) {
      printf("Error in initializing of mutex because of %s!\n", strerror(err));
      list_destroy(list);
      return NULL;
    }

    node->next = NULL;
    /* End of node's initializing */

    if (!list->first) {
      list->first = node;
      current = list->first;
    } else {
      current->next = node;
      current = current->next;
    }

    added++;
  }

  return list;
}

void list_destroy(list_t *list) {
  node_t *first = list->first;

  while (first) {
    node_t *tmp = first;

    first = first->next;

    pthread_mutex_destroy(&tmp->sync);
    free(tmp);
  }

  pthread_mutex_destroy(&list->sync);
  free(list);
}

void print_list(list_t *list) {
  node_t *current = list->first;

  while (current) {
    printf("%s\n", current->buf);
    current = current->next;
  }
}
