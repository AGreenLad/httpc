#ifndef __HC_LOG_H
#define __HC_LOG_H

typedef enum {
  _HC_LOG_DBG,
  _HC_LOG_SUCCESS,
  _HC_LOG_WARN,
  _HC_LOG_ERROR,
  _HC_LOG_FATAL,
  __HC_LOG_LVL_LEN // not for normal use
} _hc_log_level;

void _hc_log_init(const char* path, const char* filename);
void _hc_log(const char* module, _hc_log_level level, const char* format, ...);

//#define _HC_LOG_MODULE "MODULE NOT SET!"

#define LOG_DEBUG(...) _hc_log(_HC_LOG_MODULE, _HC_LOG_DBG, __VA_ARGS__)
#define LOG_SUCCESS(...) _hc_log(_HC_LOG_MODULE, _HC_LOG_SUCCESS, __VA_ARGS__)
#define LOG_WARN(...) _hc_log(_HC_LOG_MODULE, _HC_LOG_WARN, __VA_ARGS__)
#define LOG_ERROR(...) _hc_log(_HC_LOG_MODULE, _HC_LOG_ERROR, __VA_ARGS__)
#define LOG_FATAL(...) _hc_log(_HC_LOG_MODULE, _HC_LOG_FATAL, __VA_ARGS__)
#endif