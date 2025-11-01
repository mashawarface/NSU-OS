#define _GNU_SOURCE

#include <asm-generic/errno-base.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/futex.h>
#include <pthread.h>
#include <setjmp.h>
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

#include "../include/mythread.h"

static int futex(void *addr, int op, int val, const struct timespec *timeout,
                 void *addr2, int val2) {
  return syscall(SYS_futex, addr, op, val, timeout, addr2, val2);
}

detached_threads_buffer_t *buffer;

int destroy_thread(mythread_t thread) {
  if (thread == NULL || thread->stack == NULL) {
    return EINVAL;
  }

  void *stack = thread->stack;
  size_t stack_size = thread->stack_size;

  if (munmap(stack, stack_size) == -1) {
    return errno;
  }

  return 0;
}

int detached_cleanup() {
  if (buffer == NULL) {
    return 0;
  }

  for (size_t i = 0; i < buffer->counter; i++) {
    mythread_t thread = &buffer->detached_threads_list[i];
    int err = destroy_thread(thread);
    if (err != 0) {
      return 1;
    }
  }

  if (buffer->detached_threads_list != NULL) {
    free(buffer->detached_threads_list);
    buffer->detached_threads_list = NULL;
  }

  free(buffer);
  buffer = NULL;

  return 0;
}

void *create_stack(size_t size) {
  void *stack;

  stack = mmap(NULL, size, PROT_WRITE | PROT_READ, MAP_PRIVATE | MAP_ANONYMOUS,
               -1, 0);
  if (stack == MAP_FAILED) {
    return NULL;
  }

  memset(stack, 0x0, size);

  if (mprotect(stack, PAGE, PROT_NONE) == -1) {
    munmap(stack, size);
    return NULL;
  }

  return stack;
}

int mythread_startup(void *arg) {
  mythread_t thread = (mythread_t)arg;

  if (setjmp(thread->exit) == 0) {
    thread->retval = thread->start_routine(thread->args, thread);
  } else {
    thread->retval = THREAD_CANCELED;
  }

  thread->finished = 1;

  futex((unsigned long int *)&thread->finished, FUTEX_WAKE, 1, NULL, NULL, 0);

  while (!thread->joined && !thread->detached) {
    futex((void *)&thread->joined, FUTEX_WAIT, 0, NULL, NULL, 0);
  }

  if (thread->detached && thread->finished) {
    if (buffer) {
      if (buffer->counter + 1 < buffer->capasity) {
        buffer->detached_threads_list[buffer->counter++] = *thread;
      }
    } else {
      return 1;
    }
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

  usleep(100);

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

  if (!buffer) {
    buffer = malloc(sizeof(detached_threads_buffer_t));
    if (!buffer) {
      return ENOMEM;
    }
    buffer->counter = 0;
    buffer->capasity = MAX_DETACHED_THREADS;
    buffer->detached_threads_list =
        malloc(sizeof(mythread_struct_t) * buffer->capasity);
    if (!buffer->detached_threads_list) {
      return ENOMEM;
    }
  }

  thread->detached = 1;

  futex((void *)&thread->detached, FUTEX_WAKE, 1, NULL, NULL, 0);

  return 0;
}

int mythread_create(mythread_t *tid, void *(*routine)(void *, mythread_t),
                    void *args) {
  if (buffer != NULL) {
    detached_cleanup();
  }

  mythread_struct_t *thread;
  void *stack, *stack_top;

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
  thread->canceled = 0;

  stack_top = (void *)thread;

  int child_pid = clone(mythread_startup, stack_top,
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

int mythread_cancel(mythread_t thread) {
  if (!thread) {
    return ESRCH;
  }

  thread->canceled = 1;

  return 0;
}

void mythread_testcancel(mythread_t thread) {
  if (thread->canceled) {
    mythread_exit(thread);
  }
}

void mythread_exit(mythread_t thread) {
  thread->finished = 1;

  longjmp(thread->exit, 1);
}