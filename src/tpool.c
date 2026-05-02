#include "tpool.h"
#include <stdio.h>
#include <stdlib.h>

// thread pool work functions

// threadfunc_t may not be needed cuz the server only uses it to handle requests...
static _hc_tpool_work_t* _hc_tpool_work_new(_hc_threadfunc_t func, void* arg) {
  _hc_tpool_work_t* work = malloc(sizeof(_hc_tpool_work_t));
  if (work == NULL) return NULL;

  work->func = func;
  work->arg = arg;
  work->next = NULL;

  return work;
}

// only called after work is popped off the work queue or the pool is freeing,
// so no need to remove self from list as get_work does that already and 
// the whole list is getting destroyed if we're freeing the pool
static void _hc_tpool_work_free(_hc_tpool_work_t* work) {
  if (work == NULL) return;
  free(work);
}

static _hc_tpool_work_t* _hc_tpool_get_work(_hc_tpool_t* tp) {
  if (tp == NULL) return NULL;
  if (tp->work_first == NULL) return NULL;
  
  _hc_tpool_work_t* work = tp->work_first;
  
  if (work->next == NULL) { // we are the only one in the list
    tp->work_first = NULL;
    tp->work_last = NULL;
  } else {
    tp->work_first = work->next;
  }

  return work;
}

// worker function
// called for every thread in the pool, constantly checks for work or stop
static void* _hc_tpool_worker(void* arg) {
  _hc_tpool_t* tp = arg;
  _hc_tpool_work_t* work;

  for (;;) {
    pthread_mutex_lock(&tp->work_mutex);

    while (tp->work_first == NULL && !tp->stop) 
      pthread_cond_wait(&tp->work_cond, &tp->work_mutex);

    if (tp->stop) break;
    
    work = _hc_tpool_get_work(tp);
    tp->working_count++;
    pthread_mutex_unlock(&tp->work_mutex);
    
    if (work != NULL) {
      work->func(work->arg);
      _hc_tpool_work_free(work);
    }
    
    pthread_mutex_lock(&tp->work_mutex);
    tp->working_count--;

    // detect if all threads are idle
    if (!tp->stop && tp->working_count == 0 && tp->work_first == NULL)
      pthread_cond_broadcast(&tp->idle_cond);

    pthread_mutex_unlock(&tp->work_mutex);
  }

  tp->thread_count--;
  pthread_cond_broadcast(&tp->idle_cond);
  pthread_mutex_unlock(&tp->work_mutex);

  return NULL;
}

// thread pool functions
_hc_tpool_t* _hc_tpool_new(size_t thread_count) {
  _hc_tpool_t* tp = malloc(sizeof(_hc_tpool_t));

  tp->working_count = 0;
  tp->thread_count = thread_count;
  tp->stop = 0;

  pthread_mutex_init(&tp->work_mutex, NULL);
  pthread_cond_init(&tp->work_cond, NULL);
  pthread_cond_init(&tp->idle_cond, NULL);

  tp->work_first = NULL;
  tp->work_last = NULL;

  for (size_t i = 0; i < thread_count; i++) {
    pthread_t thread;
    pthread_create(&thread, NULL, _hc_tpool_worker, tp);
    pthread_detach(thread);
  }

  return tp;
}

int _hc_tpool_add_work(_hc_tpool_t* tp, _hc_threadfunc_t func, void* arg) {
  if (tp == NULL) return 0;

  _hc_tpool_work_t* work = _hc_tpool_work_new(func, arg);
  if (work == NULL) return 0;
  
  pthread_mutex_lock(&tp->work_mutex);
  if (tp->work_first == NULL) {
    tp->work_first = work;
    tp->work_last = work;
  } else {
    tp->work_last->next = work;
    tp->work_last = work;
  }

  pthread_cond_broadcast(&tp->work_cond);
  pthread_mutex_unlock(&tp->work_mutex);

  return 1;
}

// waits until all threads in a pool are idle/done with their work
void _hc_tpool_wait(_hc_tpool_t* tp) {
  if (tp == NULL) return;

  pthread_mutex_lock(&tp->work_mutex);

  for (;;) {
    // ^ in case of random cond broadcasts
    if (tp->work_first != NULL || tp->working_count > 0) {
      pthread_cond_wait(&tp->idle_cond, &tp->work_mutex);
    } else {
      break;
    }
  }

  pthread_mutex_unlock(&tp->work_mutex);
}

// gracefully shuts down the pool and frees related items (mutexes, conditions, work left in the queue)
// make immediate shutdown function?
void _hc_tpool_free(_hc_tpool_t* tp) {
  if (tp == NULL) 
    return;

  // lock to make sure we aren't interrupting some thread
  pthread_mutex_lock(&tp->work_mutex);

  // destroy work queue
  _hc_tpool_work_t* work = tp->work_first;
  while (work != NULL) {
    _hc_tpool_work_t* next_work = work->next;
    _hc_tpool_work_free(work);
    work = next_work;
  }
  tp->work_first = NULL;

  // get every thread to stop and wait to make sure they are done
  tp->stop = 1;
  pthread_cond_broadcast(&tp->work_cond);
  pthread_mutex_unlock(&tp->work_mutex);
  _hc_tpool_wait(tp);

  // free every mutex and self
  pthread_mutex_destroy(&tp->work_mutex);
  pthread_cond_destroy(&tp->work_cond);
  pthread_cond_destroy(&tp->idle_cond);

  free(tp); // the pool should NEVER be on stack
}