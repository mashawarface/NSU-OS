#define _GNU_SOURCE

#include "../include/mythread.h"

static int destroy_thread(mythread_t thread) {
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

  while (!thread->joined) {
    usleep(100);
  }

  return 0;
}

int mythread_create(mythread_t *tid, void *(*routine)(void *, mythread_t),
                    void *args) {

  mythread_struct_t *thread;
  void *stack, *stack_top;

  stack = create_stack(STACK_SIZE);
  if (!stack) {
    return errno;
  }

  thread =
      (mythread_struct_t *)(stack + STACK_SIZE - sizeof(mythread_struct_t));
  thread->thread_id = (unsigned long)thread;
  thread->start_routine = routine;
  thread->args = args;
  thread->retval = NULL;
  thread->stack = stack;
  thread->stack_size = STACK_SIZE;
  thread->joined = 0;
  thread->finished = 0;
  thread->canceled = 0;

  stack_top = (void *)thread;

  int child_pid =
      clone(mythread_startup, stack_top,
            CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_THREAD,
            (void *)thread);
  if (child_pid == -1) {
    destroy_thread(thread);
    return errno;
  }

  *tid = thread;

  return 0;
}

size_t mythread_self(mythread_t thread) { return thread->thread_id; }

int mythread_equal(mythread_t thread1, mythread_t thread2) {
  return thread1 == thread2;
}

int mythread_join(mythread_t thread, void **retval) {
  if (thread == NULL) {
    return ESRCH;
  }

  while (!thread->finished) {
    usleep(100);
  }

  if (retval != NULL) {
    *retval = thread->retval;
  }

  thread->joined = 1;

  usleep(1000);

  int err = destroy_thread(thread);
  if (err != 0) {
    return err;
  }

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
    longjmp(thread->exit, 1);
  }
}
