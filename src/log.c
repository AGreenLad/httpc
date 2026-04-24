#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <time.h>
#include "log.h"

static const char* log_lvl_strs[__HC_LOG_LVL_LEN] = {
  "DEBUG", "SUCCESS", "WARN", "ERROR", "FATAL"
};

static const char* log_lvl_colors[__HC_LOG_LVL_LEN] = {
  "\x1b[39m", "\x1b[32;1m", "\x1b[33;1m", "\x1b[31;1m", "\x1b[30;47;1m"
};


void _hc_log(_hc_log_level level, const char* format, ...) {
  va_list args;
  va_start(args, format);

  time_t curr_time = time(NULL);
  struct tm* tm = localtime(&curr_time);

  // set the color
  printf("%s", log_lvl_colors[level]);
  // print time
  printf("[%02d:%02d:%02d]",
    tm->tm_hour,
    tm->tm_min,
    tm->tm_sec
  );
  // print log level
  printf("[%s] ", log_lvl_strs[level]);
  // print the actual log
  vfprintf(stdout, format, args);
  puts("\x1b[0m"); // reset our colors or shell will be all messed up
}