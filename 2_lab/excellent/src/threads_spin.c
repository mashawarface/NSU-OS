#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "../lib/include/list_spin.h"

#define THREAD_COUNT 6
#define SEARCH_THREAD_COUNT 3
#define SWAP_THREAD_COUNT 3

void signal_handler(int sig) {
  char *message = "\nReceived signal! Terminate the process...\n";
  write(1, message, strlen(message));
}

int compare_ascend(int curr, int prev) { return curr > prev; }

int compare_descend(int curr, int prev) { return curr < prev; }

int compare_equal(int curr, int prev) { return curr == prev; }

void *search(void *arg) {
  int err;

  search_thread_arg_t *thread_arg = (search_thread_arg_t *)arg;
  list_t *list = thread_arg->list;
  size_t *iterations = thread_arg->counter, counter = 0;
  int (*compare)(int, int) = thread_arg->compare;

  if (!list) {
    printf("List points to NULL!");
    return NULL;
  }

  // Look at list's head safety
  err = pthread_spin_lock(&list->sync);
  if (err) {
    printf("Can't lock spin bc of %s!\n", strerror(err));
    return NULL;
  }

  node_t *current = list->first;

  pthread_spin_unlock(&list->sync);

  int prev_length = 0, curr_length;

  while (1) {

    /* Set cancel state = DISABLED, bc we don't want our thread cancel in the
     middle of iteration.
     From the man: None of the mutex functions is a
     cancellation point, not even pthread_mutex_lock, in spite of the fact that
     it can suspend a thread for arbitrary durations. So, does we even need
     pthread_setcancelstate?
     If there is some error in locking, we print it out using printf(), which is
     cancellation point.
     But if we in error branch, what is the problem with cancel our thread in
     this point? Ask seminarist about this ... */

    err = pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    if (err) {
      printf("Can't set PTHREAD_CANCEL_DISABLE state bc of %s!", strerror(err));
      return NULL;
    }

    err = pthread_spin_lock(&current->sync);
    if (err) {
      printf("Can't lock spin bc of %s!\n", strerror(err));
      break;
    }

    curr_length = strlen(current->buf);

    (compare(curr_length, prev_length)) ? (counter++) : (counter = 0);

    prev_length = curr_length;

    pthread_spin_unlock(&current->sync);

    current = current->next;

    if (current == NULL) {
      (*iterations)++;

      err = pthread_spin_lock(&list->sync);
      if (err) {
        printf("Can't lock spin bc of %s!\n", strerror(err));
        return NULL;
      }

      current = list->first;

      pthread_spin_unlock(&list->sync);
    }

    err = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    if (err) {
      printf("Can't set PTHREAD_CANCEL_ENABLE state bc of %s!", strerror(err));
      return NULL;
    }

    // Set cancel point, bc we do not have other cancel point.
    pthread_testcancel();
  }

  return NULL;
}

void *swap(void *arg) {
  int err;

  swap_thread_arg_t *thread_arg = (swap_thread_arg_t *)arg;
  list_t *list = thread_arg->list;
  size_t *counter = thread_arg->counter;

  if (!list) {
    printf("List points to NULL!");
    return NULL;
  }

  while (1) {
    node_t *prev = NULL, *curr = NULL, *next = NULL;

    int prob_counter = 0, make_swap = 0;

    while (1) {
      err = pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
      if (err) {
        printf("Can't set PTHREAD_CANCEL_DISABLE state bc of %s!",
               strerror(err));
        return NULL;
      }

      make_swap = (prob_counter > 3);

      if (!make_swap) {
        prob_counter = (prob_counter + 1) % 10;
        continue;
      }

      if (!prev) {
        err = pthread_spin_lock(&list->sync);
        if (err) {
          printf("Can't lock spin bc of %s!\n", strerror(err));
          break;
        }

        node_t *first = list->first;
        if (!first || !first->next) {
          break;
        }

        err = pthread_spin_lock(&first->sync);
        if (err) {
          pthread_spin_unlock(&list->sync);
          printf("Can't lock spin bc of %s!\n", strerror(err));
          break;
        }

        curr = first;
        if (!curr->next) {
          pthread_spin_unlock(&list->sync);
          pthread_spin_unlock(&curr->sync);
          break;
        }

        next = curr->next;
        err = pthread_spin_lock(&next->sync);
        if (err) {
          pthread_spin_unlock(&list->sync);
          pthread_spin_unlock(&curr->sync);
          printf("Can't lock spin bc of %s!\n", strerror(err));

          break;
        }

        if (curr->next != next) {
          pthread_spin_unlock(&list->sync);
          pthread_spin_unlock(&curr->sync);
          pthread_spin_unlock(&next->sync);

          break;
        }

        list->first = next;
        curr->next = next->next;
        next->next = curr;

        (*counter)++;

        prev = curr;

        prob_counter = (prob_counter + 1) % 10;

        pthread_spin_unlock(&list->sync);
        pthread_spin_unlock(&curr->sync);
        pthread_spin_unlock(&next->sync);
      } else {
        err = pthread_spin_lock(&prev->sync);
        if (err) {
          printf("Can't lock spin bc of %s!\n", strerror(err));
          break;
        }

        if (!prev->next || !prev->next->next) {
          pthread_spin_unlock(&prev->sync);
          prev = NULL;
          break;
        }

        curr = prev->next;
        err = pthread_spin_lock(&curr->sync);
        if (err) {
          pthread_spin_unlock(&prev->sync);
          printf("Can't lock spin bc of %s!\n", strerror(err));
          break;
        }

        next = curr->next;
        err = pthread_spin_lock(&next->sync);
        if (err) {
          pthread_spin_unlock(&curr->sync);
          pthread_spin_unlock(&prev->sync);
          printf("Can't lock spin bc of %s!\n", strerror(err));
          break;
        }

        if (prev->next != curr || curr->next != next) {
          pthread_spin_unlock(&next->sync);
          pthread_spin_unlock(&curr->sync);
          pthread_spin_unlock(&prev->sync);

          break;
        }

        prev->next = next;
        curr->next = next->next;
        next->next = curr;

        (*counter)++;

        node_t *tmp = prev;
        prev = curr;

        prob_counter = (prob_counter + 1) % 10;

        pthread_spin_unlock(&tmp->sync);
        pthread_spin_unlock(&curr->sync);
        pthread_spin_unlock(&next->sync);
      }

      err = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
      if (err) {
        printf("Can't set PTHREAD_CANCEL_ENABLE state bc of %s!",
               strerror(err));
        return NULL;
      }

      pthread_testcancel();
    }

    err = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    if (err) {
      printf("Can't set PTHREAD_CANCEL_ENABLE state bc of %s!", strerror(err));
      return NULL;
    }

    pthread_testcancel();
  }
}

void print_statistic(size_t *counters) {
  printf("List stats:\n"
         "\tThread (ascending pairs):  %zu iteration(s)\n"
         "\tThread (descending pairs): %zu iteration(s)\n"
         "\tThread (equal pairs):      %zu iteration(s)\n"
         "\tFisrt swapping thread:     %zu successful swap(s)\n"
         "\tSecond swapping thread:    %zu successful swap(s)\n"
         "\tThird swapping thread:     %zu successful swap(s)\n",
         counters[0], counters[1], counters[2], counters[3], counters[4],
         counters[5]);
}

int main(int argc, char **argv) {
  // srand(time(0));

  if (argc != 2) {
    printf("Usage: %s <list_size>\n", argv[0]);

    return 1;
  }

  int list_size = atoi(argv[1]);
  list_t *list = list_init(list_size);

  if (list == NULL) {
    printf("List is not initialized!\n");
    return 1;
  }

  int err;

  // Fill mask, so children threads won't react on signals
  sigset_t thread_mask;

  err = sigfillset(&thread_mask);
  if (err == -1) {
    printf("Can't fill mask bc of %s!\n", strerror(errno));
    return 1;
  }

  err = pthread_sigmask(SIG_BLOCK, &thread_mask, NULL);
  if (err) {
    printf("Can't set mask bc of %s!\n", strerror(err));
    return 1;
  }

  pthread_t tids[THREAD_COUNT];

  size_t counters[THREAD_COUNT] = {0, 0, 0, 0, 0, 0};

  search_thread_arg_t search_args[SEARCH_THREAD_COUNT] = {
      {list, &counters[0], compare_ascend},
      {list, &counters[1], compare_descend},
      {list, &counters[2], compare_equal}};

  swap_thread_arg_t swap_args[SWAP_THREAD_COUNT] = {
      {list, &counters[3]}, {list, &counters[4]}, {list, &counters[5]}};

  int thread_num = 0;

  for (; thread_num < SEARCH_THREAD_COUNT; thread_num++) {
    err = pthread_create(&tids[thread_num], NULL, search,
                         &search_args[thread_num]);
    if (err != 0) {
      printf("Error in creating thread #%d because of %s!\n", thread_num,
             strerror(err));
      return 1;
    }
  }

  for (; thread_num < THREAD_COUNT; thread_num++) {
    err = pthread_create(&tids[thread_num], NULL, swap,
                         &swap_args[thread_num - 3]);
    if (err != 0) {
      printf("Error in creating thread #%d because of %s!\n", thread_num,
             strerror(err));
      return 1;
    }
  }

  // Unblock SIGINT
  sigset_t main_mask;

  err = sigaddset(&main_mask, SIGINT);
  if (err == -1) {
    printf("Can't fill mask bc of %s!\n", strerror(errno));
    return 1;
  }

  err = pthread_sigmask(SIG_UNBLOCK, &main_mask, &thread_mask);
  if (err) {
    printf("Can't set mask bc of %s!\n", strerror(err));
    return 1;
  }

  struct sigaction act;
  memset(&act, 0, sizeof(struct sigaction));
  act.sa_handler = signal_handler;
  act.sa_flags = SA_RESTART;

  err = sigaction(SIGINT, &act, NULL);
  if (err != 0) {
    printf("Error in sigaction because of %s!\n", strerror(err));
    return 1;
  }

  // Wait until there is SIGINT
  pause();

  // Bc our threads have infty cycle while(1), we can't just join it, so cancel
  // all of it.
  for (int i = 0; i < THREAD_COUNT; i++) {
    err = pthread_cancel(tids[i]);
  }

  print_statistic(counters);

  for (int i = 0; i < THREAD_COUNT; i++) {
    err = pthread_join(tids[i], NULL);
    if (err != 0) {
      printf("Can`t join thread #%d because of %s!\n", i, strerror(err));
      return 1;
    }
  }

  list_destroy(list);

  return 0;
}
