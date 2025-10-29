#define mythread_cleanup_push(routine, arg)
#define mythread_cleanup_pop(execute)

typedef struct mythread_t {
  unsigned long int tid;
} mythread_t;

typedef struct mythread_cleanup_buffer {
} mythread_cleanup_buffer;

int mythread_create(mythread_t newthread, void *(*routine)(void *),
                    void *arguments);

void mythread_exit(void *thread_return);

mythread_t mythread_self(void);

int mythread_equal(mythread_t thread1, mythread_t thread2);

int mythread_join(mythread_t thread, void **thread_return);

int mythread_detach(mythread_t thread);

int mythread_cancel(mythread_t thread);

int mythread_testcancel(void);
