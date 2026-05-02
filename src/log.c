#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>
#include "log.h"

#define _HC_LOG_MODULE "LOG"

static FILE* log_file = NULL;
static pthread_mutex_t logging_mutex = PTHREAD_MUTEX_INITIALIZER;

static const char* log_lvl_strs[__HC_LOG_LVL_LEN] = {
  "INFO", "SUCCESS", "WARN", "ERROR", "FATAL"
};

static const char* log_lvl_colors[__HC_LOG_LVL_LEN] = {
  "\x1b[39m", "\x1b[32;1m", "\x1b[33;1m", "\x1b[31;1m", "\x1b[30;47;1m"
};

static char* find_next_log_name(const char* path, const char* name) {
  int log_count = 1;
  size_t filename_len = strlen(path) + strlen(name) + 12; // name + -000000.log + null terminator
  char* log_name = calloc(filename_len, sizeof(char));
  FILE* f;

  snprintf(log_name, filename_len, "%s%s-%.5d.log", path, name, log_count);
  while ((f = fopen(log_name, "r")) != NULL) {
    fclose(f);
    if (log_count++ > 999999) { 
      LOG_WARN("Couldn't find log name, too many logs!");
      free(log_name);
      return NULL;
    }
    snprintf(log_name, filename_len, "%s%s-%.5d.log", path, name, log_count);
  }
  return log_name;
}

void _hc_log_init(const char* path, const char* filename) {
  pthread_mutex_lock(&logging_mutex);
  char* next_log_name;

  if (log_file && log_file != stdout && log_file != stderr) fclose(log_file);

  if (path == NULL || filename == NULL) log_file = stdout;
  else {
    next_log_name = find_next_log_name(path, filename);
    if (next_log_name == NULL) {
      puts("Error while initializing log system: Couldn't find another log name");
    }
    log_file = fopen(next_log_name, "a");
    if (!log_file) {
      printf("Error while initializing log system: Failed to open log file %s%s: %s\n", path, filename, strerror(errno));
      log_file = stdout;
    } else {
      setvbuf(log_file, NULL, _IONBF, 0);
    }
  }

  pthread_mutex_unlock(&logging_mutex);
  LOG_DEBUG("Logger initialized, File: %s", log_file != stdout ? next_log_name : "none, using stdout");
}

void _hc_log(const char* module, _hc_log_level level, const char* format, ...) {
  // multiple threads are gonna be logging so this is needed
  pthread_mutex_lock(&logging_mutex);

  va_list args;
  va_start(args, format);

  time_t curr_time = time(NULL);
  struct tm* tm = localtime(&curr_time);

  // set the color if we're using stdout
  if (log_file == stdout) fprintf(log_file, "%s", log_lvl_colors[level]);
  // print time, log level, and module
  fprintf(log_file, "[%02d:%02d:%02d] [%s] [%s] ",
    tm->tm_hour,
    tm->tm_min,
    tm->tm_sec,
    log_lvl_strs[level],
    module
  );
  
  //pthread_t thread_id = pthread_self();
  // fprintf(log_file, "[TID: %lu] ", (unsigned long) thread_id);

  // print the actual log
  vfprintf(log_file, format, args);
  if (log_file == stdout) puts("\x1b[0m"); // if logging to stdout, reset our colors or shell will be all messed up
  else fprintf(log_file, "\n");

  pthread_mutex_unlock(&logging_mutex);
}