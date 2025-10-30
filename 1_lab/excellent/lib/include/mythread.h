#define PAGE 4096
#define STACK_SIZE PAGE * 8

#define mythread_cleanup_push(routine, arg)
#define mythread_cleanup_pop(execute)

// typedef unsigned long int mythread_t;

typedef void *(*start_routine_t)(void *);

typedef struct mythread {
  unsigned long int   thread_id;
  start_routine_t     start_routine;
  void*               args;
  void*               retval;
  void*               stack;
  int                 stack_size;
  int                 joined;
  int                 exited;
} mythread_t;

typedef struct mythread_cleanup_buffer {
} mythread_cleanup_buffer;

int mythread_create(mythread_t *thread, void *(*routine)(void *), void *args);

void mythread_exit(void *retval);

unsigned long int mythread_self(void);

int mythread_equal(mythread_t thread1, mythread_t thread2);

int mythread_join(mythread_t thread, void **retval);

int mythread_detach(mythread_t thread);

int mythread_cancel(mythread_t thread);

int mythread_testcancel(void);
