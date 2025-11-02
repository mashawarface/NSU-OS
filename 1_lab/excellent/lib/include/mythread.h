#ifndef _MY_THREAD_H_
#define _MY_THREAD_H_

#include <errno.h>
#include <linux/futex.h>
#include <sched.h>
#include <setjmp.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <unistd.h>

#define PAGE 4096
#define STACK_SIZE PAGE * 8
#define MAX_DETACHED_THREADS 10
#define THREAD_CANCELED ((void *)-1)

typedef struct mythread {
  unsigned long thread_id;
  void *(*start_routine)(void *, struct mythread *);
  void *args;
  void *retval;
  void *stack;
  int stack_size;
  volatile int joined;
  volatile int finished;
  volatile int canceled;
  jmp_buf exit;
} mythread_struct_t;

typedef mythread_struct_t *mythread_t;

typedef void *(*routine_t)(void *, mythread_t);

int mythread_create(mythread_t *thread, routine_t routine, void *args);

size_t mythread_self(mythread_t thread);

int mythread_equal(mythread_t thread1, mythread_t thread2);

int mythread_join(mythread_t thread, void **retval);

int mythread_cancel(mythread_t thread);

void mythread_testcancel(mythread_t thread);

#endif // _MY_THREAD_H_