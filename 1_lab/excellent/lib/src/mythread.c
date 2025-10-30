#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>

#include "/home/mary/NSU-OS/1_lab/excellent/lib/include/mythread.h"

void *create_stack(size_t size) {
  void *stack;

  stack = mmap(NULL, size, PROT_WRITE | PROT_READ, MAP_PRIVATE | MAP_ANONYMOUS,
               -1, 0);
  if (stack == MAP_FAILED) {
    printf("Mapping is failed, %s!\n", strerror(errno));
    exit(1);
  }

  if (mprotect(stack, PAGE, PROT_NONE) == -1) {
    printf("Changing protection is failed, %s!\n", strerror(errno));
    exit(1);
  }

  return stack;
}

int mythread_startup(void *arg) {
  puts("!");

  mythread_t *thread = (mythread_t *)arg;

  thread->retval = thread->start_routine(thread->args);
  thread->exited = 1;

  while (1) {
    sleep(10);
  }

  //   return 0;
}

int mythread_create(mythread_t *tid, void *(*routine)(void *), void *args) {
  void *stack;
  size_t stack_size = STACK_SIZE - sizeof(mythread_t);
  mythread_t *thread;

  stack = create_stack(STACK_SIZE);

  thread = (mythread_t *)(stack + stack_size);
  thread->thread_id = (unsigned long int)thread;
  thread->start_routine = routine;
  thread->args = args;
  thread->stack = stack;
  thread->stack_size = stack_size + sizeof(mythread_t);
  thread->joined = 0;
  thread->exited = 0;

  int child_pid;
  child_pid = clone(mythread_startup, stack + stack_size,
                    CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND |
                        CLONE_THREAD | CLONE_SYSVSEM,
                    (void *)thread);
  if (child_pid == -1) {
    printf("Clone is failed, %s!\n", strerror(errno));
    return 1;
  }

  tid = thread;

  return 0;
}
