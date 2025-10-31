#define _GNU_SOURCE

#include <asm-generic/errno-base.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/futex.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "/home/mary/NSU-OS/1_lab/excellent/lib/include/mythread.h"

static int futex(void *addr, int op, int val, const struct timespec *timeout,
                 void *addr2, int val2) {
  return syscall(SYS_futex, addr, op, val, timeout, addr2, val2);
}

void *create_stack(size_t size) {
  void *stack;

  stack = mmap(NULL, size, PROT_WRITE | PROT_READ, MAP_PRIVATE | MAP_ANONYMOUS,
               -1, 0);
  if (stack == MAP_FAILED) {
    return NULL;
  }

  if (mprotect(stack, PAGE, PROT_NONE) == -1) {
    munmap(stack, size);
    return NULL;
  }

  return stack;
}

int destroy_thread(mythread_t thread) {
  if (munmap(thread->stack, thread->stack_size) == -1) {
    return errno;
  }

  return 0;
}

int mythread_startup(void *arg) {
  mythread_t thread = (mythread_t)arg;

  thread->retval = thread->start_routine(thread->args);
  thread->finished = 1;

  futex((unsigned long int *)&thread->finished, FUTEX_WAKE, 1, NULL, NULL, 0);

  while (!thread->joined && !thread->detached) {
    futex((void *)&thread->joined, FUTEX_WAIT, 0, NULL, NULL, 0);
    futex((void *)&thread->detached, FUTEX_WAIT, 0, NULL, NULL, 0);
  }

  return 0;
}

int mythread_join(mythread_t thread, void **retval) {
  if (thread == NULL) {
    return ESRCH;
  }

  if (thread->detached) {
    return EINVAL;
  }

  while (!thread->finished) {
    futex((void *)&thread->finished, FUTEX_WAIT, 0, NULL, NULL, 0);
  }

  if (retval != NULL) {
    *retval = thread->retval;
  }

  thread->joined = 1;

  futex((void *)&thread->joined, FUTEX_WAKE, 1, NULL, NULL, 0);

  destroy_thread(thread);

  return 0;
}

int mythread_detach(mythread_t thread) {
  if (!thread) {
    return ESRCH;
  }

  if (thread->detached) {
    return EINVAL;
  }

  thread->detached = 1;

  futex((void *)&thread->detached, FUTEX_WAKE, 1, NULL, NULL, 0);

  return 0;
}

int mythread_create(mythread_t *tid, void *(*routine)(void *), void *args) {
  mythread_struct_t *thread;
  void *stack;

  stack = create_stack(STACK_SIZE);
  if (!stack) {
    return errno;
  }

  thread =
      (mythread_struct_t *)(stack + STACK_SIZE - sizeof(mythread_struct_t));
  thread->thread_id = (unsigned long int)thread;
  thread->start_routine = routine;
  thread->args = args;
  thread->retval = NULL;
  thread->stack = stack;
  thread->stack_size = STACK_SIZE;
  thread->joined = 0;
  thread->detached = 0;
  thread->finished = 0;

  stack = (void *)thread;

  int child_pid = clone(mythread_startup, stack,
                        CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND |
                            CLONE_THREAD | CLONE_SYSVSEM,
                        (void *)thread);
  if (child_pid == -1) {
    destroy_thread(thread);
    return errno;
  }

  *tid = thread;

  return 0;
}
