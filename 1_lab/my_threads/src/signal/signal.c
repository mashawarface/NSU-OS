#include <bits/types/sigset_t.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define THREAD_COUNT 3

void signal_handler(int sig) {
  (void)sig;
  // char *message = "\nReceived SIGINT!\n";
  // write(1, message, strlen(message));
  printf("This signal accepted by %lu thread! Continue working after "
         "receiving signal %s!\n",
         pthread_self(), strsignal(sig));
}

void *thread_ignore_all(void *arg) {
  (void)arg;
  int err;

  printf("Thread, that is ignored all signal: %lu\n", pthread_self());

  sigset_t mask;

  err = sigfillset(&mask);
  if (err != 0) {
    printf("Error in sigfillset because of %s!\n", strerror(err));
    return NULL;
  }

  pthread_sigmask(SIG_BLOCK, &mask, NULL);
  if (err != 0) {
    printf("Error in setting sig mask because of %s!\n", strerror(err));
    return NULL;
  }

  while (1) {
    pause();
    printf("This signal accepted by %lu thread! Continue working after "
           "receiving signal!\n",
           pthread_self());
  }

  return NULL;
}

void *thread_sigint(void *arg) {
  (void)arg;
  int err;

  printf("Thread, that is accepted sigint: %lu\n", pthread_self());

  sigset_t mask;

  err = sigaddset(&mask, SIGINT);
  if (err != 0) {
    printf("Error in sigaddset because of %s!\n", strerror(err));
    return NULL;
  }

  pthread_sigmask(SIG_UNBLOCK, &mask, NULL);
  if (err != 0) {
    printf("Error in setting sig mask because of %s!\n", strerror(err));
    return NULL;
  }

  struct sigaction act;
  act.sa_handler = signal_handler;
  act.sa_flags = SA_RESTART;

  err = sigemptyset(&act.sa_mask);
  if (err != 0) {
    printf("Error in sigemptyset because of %s!\n", strerror(err));
    return NULL;
  }

  err = sigaction(SIGINT, &act, NULL);
  if (err != 0) {
    printf("Error in sigaction because of %s!\n", strerror(err));
    return NULL;
  }

  while (1) {
    pause();
  }

  return NULL;
}

void *thread_sigquit(void *arg) {
  (void)arg;
  int err, sig;

  printf("Thread, that is accepted sigquit: %lu\n", pthread_self());

  sigset_t mask;

  err = sigaddset(&mask, SIGQUIT);
  if (err != 0) {
    printf("Error in sigaddset because of %s!\n", strerror(err));
    return NULL;
  }

  pthread_sigmask(SIG_UNBLOCK, &mask, NULL);
  if (err != 0) {
    printf("Error in setting sig mask because of %s!\n", strerror(err));
    return NULL;
  }

  while (1) {
    err = sigwait(&mask, &sig);

    if (err != 0) {
      printf("Error in sigwait beacuse of %s!\n", strerror(err));
      return NULL;
    }

    printf("This signal accepted by %lu thread! Continue working after "
           "receiving signal %s!\n",
           pthread_self(), strsignal(sig));
  }

  return NULL;
}

int main() {
  pthread_t tids[THREAD_COUNT];
  int err;

  // start setting mask
  sigset_t main_mask;

  err = sigfillset(&main_mask);
  if (err != 0) {
    printf("Error in sigemptyset because of %s!\n", strerror(err));
    return 1;
  }

  err = pthread_sigmask(SIG_BLOCK, &main_mask, NULL);
  if (err != 0) {
    printf("Error in setting sig mask because of %s!\n", strerror(err));
    return 1;
  }
  // end setting mask

  printf("PID: %d\n", getpid());

  err = pthread_create(&tids[0], NULL, thread_ignore_all, NULL);
  if (err != 0) {
    printf("Error in creating thread #1 because of %s!\n", strerror(err));
    return 1;
  }

  err = pthread_create(&tids[1], NULL, thread_sigint, NULL);
  if (err != 0) {
    printf("Error in creating thread #2 because of %s!\n", strerror(err));
    return 1;
  }

  err = pthread_create(&tids[2], NULL, thread_sigquit, NULL);
  if (err != 0) {
    printf("Error in creating thread #3 because of %s!\n", strerror(err));
    return 1;
  }

  for (int i = 0; i < THREAD_COUNT; i++) {
    pthread_join(tids[i], NULL);
    if (err != 0) {
      printf("Can`t join thread #%d because of %s!\n", i, strerror(err));
      return 1;
    }
  }

  return 0;
}
