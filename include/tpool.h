#include <pthread.h>
#include <stddef.h>

typedef void (*_hc_threadfunc_t)(void* arg);

typedef struct _hc_tpool_work {
  _hc_threadfunc_t func;
  void* arg;
  struct _hc_tpool_work* prev;
  struct _hc_tpool_work* next;
} _hc_tpool_work_t;


typedef struct _hc_tpool {
  // work queue as linked list
  _hc_tpool_work_t* work_first;
  _hc_tpool_work_t* work_last;
  pthread_mutex_t work_mutex; // mutex for the whole pool
  pthread_cond_t work_cond; // signaled when work is in the queue
  pthread_cond_t idle_cond; // signaled when a thread dies or when a thread detects idleness
  size_t working_count;
  size_t thread_count;
  int stop; // set when threads should stop
} _hc_tpool_t;


_hc_tpool_t* _hc_tpool_new(size_t thread_count);
void _hc_tpool_free(_hc_tpool_t* tp);

int _hc_tpool_add_work(_hc_tpool_t* tp, _hc_threadfunc_t func, void* arg);
void _hc_tpool_wait(_hc_tpool_t* tp);

