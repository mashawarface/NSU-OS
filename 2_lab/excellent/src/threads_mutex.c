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

#include "../lib/include/list_mutex.h"

#define THREAD_COUNT 6
#define SEARCH_THREAD_COUNT 3
#define SWAP_THREAD_COUNT 3

void signal_handler(int sig) {
  char *message =
      "\nReceived SIGINT! Let's terminate the process correctly...\n";
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

  // Look at list's head safety.
  err = pthread_mutex_lock(&list->sync);
  if (err) {
    printf("Can't lock mutex bc of %s!\n", strerror(err));

    return NULL;
  }

  node_t *current = list->first;

  pthread_mutex_unlock(&list->sync);

  int prev_length = 0, curr_length = 0;

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
     this point? Ask seminarist about this ...
     But what if someone make cancel asynchronous? If that,
     pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL) makes sence. */
    err = pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    if (err) {
      printf("Can't set PTHREAD_CANCEL_DISABLE state bc of %s!", strerror(err));

      return NULL;
    }

    err = pthread_mutex_lock(&current->sync);
    if (err) {
      printf("Can't lock mutex bc of %s!\n", strerror(err));

      break;
    }

    curr_length = strlen(current->buf);

    (compare(curr_length, prev_length)) ? (counter++) : (counter = 0);

    prev_length = curr_length;

    pthread_mutex_unlock(&current->sync);

    current = current->next;

    if (current == NULL) {
      (*iterations)++;

      err = pthread_mutex_lock(&list->sync);
      if (err) {
        printf("Can't lock mutex bc of %s!\n", strerror(err));

        return NULL;
      }

      current = list->first;

      pthread_mutex_unlock(&list->sync);
    }

    err = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    if (err) {
      printf("Can't set PTHREAD_CANCEL_ENABLE state bc of %s!", strerror(err));

      return NULL;
    }

    // Set cancellation point, bc we do not have other cancellation points.
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
        err = pthread_mutex_lock(&list->sync);
        if (err) {
          printf("Can't lock mutex bc of %s!\n", strerror(err));

          break;
        }

        node_t *first = list->first;
        if (!first || !first->next) {
          pthread_mutex_unlock(&list->sync);

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

        prob_counter = (prob_counter + 1) % 10;

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

        prob_counter = (prob_counter + 1) % 10;

        pthread_mutex_unlock(&tmp->sync);
        pthread_mutex_unlock(&curr->sync);
        pthread_mutex_unlock(&next->sync);
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
  printf("\033[42mList stats:\033[0m\n"
         "Thread (ascending pairs):  %zu\n"
         "Thread (descending pairs): %zu\n"
         "Thread (equal pairs):      %zu\n"
         "Fisrt swapping thread:     %zu\n"
         "Second swapping thread:    %zu\n"
         "Third swapping thread:     %zu\n",
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

  // Fill mask, so children threads won't react on signals.
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

  // Create three search threads.
  for (; thread_num < SEARCH_THREAD_COUNT; thread_num++) {
    err = pthread_create(&tids[thread_num], NULL, search,
                         &search_args[thread_num]);
    if (err != 0) {
      printf("Error in creating thread #%d because of %s!\n", thread_num,
             strerror(err));

      return 1;
    }
  }

  // Create three swap threads.
  for (; thread_num < THREAD_COUNT; thread_num++) {
    err = pthread_create(&tids[thread_num], NULL, swap,
                         &swap_args[thread_num - SEARCH_THREAD_COUNT]);
    if (err != 0) {
      printf("Error in creating thread #%d because of %s!\n", thread_num,
             strerror(err));

      return 1;
    }
  }

  // Unblock SIGINT.
  sigset_t main_mask;

  err = sigemptyset(&main_mask);
  if (err == -1) {
    printf("Can't set mask empty bc of %s!\n", strerror(errno));

    return 1;
  }

  err = sigaddset(&main_mask, SIGINT);
  if (err == -1) {
    printf("Can't add SIGINT to mask bc of %s!\n", strerror(errno));

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
  if (err == -1) {
    printf("Error in sigaction because of %s!\n", strerror(err));

    return 1;
  }

  // Wait until there is SIGINT.
  pause();

  // Bc our threads have infty cycle while(1), we can't just join it, so cancel
  // all of it.
  for (int i = 0; i < THREAD_COUNT; i++) {
    err = pthread_cancel(tids[i]);
    if (err) {
      printf("Can`t cancel thread #%d because of %s!\n", i, strerror(err));

      return 1;
    }
  }

  print_statistic(counters);

  for (int i = 0; i < THREAD_COUNT; i++) {
    err = pthread_join(tids[i], NULL);
    if (err) {
      printf("Can`t join thread #%d because of %s!\n", i, strerror(err));

      return 1;
    }
  }

  list_destroy(list);

  return 0;
}
