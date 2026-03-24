#include <pthread.h>
#include <stddef.h>

typedef void (*threadfunc_t)(void* arg);

struct tpool_work {
  threadfunc_t func;
  void* arg;
  struct tpool_work* prev;
  struct tpool_work* next;
};

typedef struct tpool_work tpool_work_t;

struct tpool {
  // work queue as linked list
  tpool_work_t* work_first;
  tpool_work_t* work_last;
  pthread_mutex_t work_mutex; // mutex for the whole pool
  pthread_cond_t work_cond; // signaled when work is in the queue
  pthread_cond_t idle_cond; // signaled when a thread dies or when a thread detects idleness
  size_t working_count;
  size_t thread_count;
  int stop; // set when threads should stop
};

typedef struct tpool tpool_t;

tpool_t* tpool_new(size_t thread_count);
void tpool_free(tpool_t* tp);

int tpool_add_work(tpool_t* tp, threadfunc_t func, void* arg);
void tpool_wait(tpool_t* tp);

