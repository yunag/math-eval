#ifndef MATH_EVAL_LOG_H
#define MATH_EVAL_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

enum math_eval_log_severity {
  MATH_EVAL_SEVERITY_INFO = 0,
  MATH_EVAL_SEVERITY_WARN,
  MATH_EVAL_SEVERITY_ERROR,
};

typedef void (*math_eval_message_handler)(enum math_eval_log_severity severity,
                                          const char *msg);

void math_eval_install_message_handler(math_eval_message_handler handler);
void math_eval_log_message(enum math_eval_log_severity severity,
                           const char *msg);

#define MATH_EVAL_STR(x) MATH_EVAL_STR2(x)
#define MATH_EVAL_STR2(x) #x

#ifdef NDEBUG
#define MATH_EVAL_LOG_DEBUG
#else
#define MATH_EVAL_LOG_DEBUG __FILE__ ":" MATH_EVAL_STR(__LINE__) ": "
#endif /* NDEBUG */

#define MATH_EVAL_LOG(severity, ...)                                           \
  do {                                                                         \
    char message[1024];                                                        \
    snprintf(message, sizeof(message), MATH_EVAL_LOG_DEBUG __VA_ARGS__);       \
    math_eval_log_message(severity, message);                                  \
  } while (0)

#ifndef MATH_EVAL_NOLOG
#include <stdio.h>
#define MATH_EVAL_LOG_ERROR(...)                                               \
  MATH_EVAL_LOG(MATH_EVAL_SEVERITY_ERROR, __VA_ARGS__)
#define MATH_EVAL_LOG_WARN(...)                                                \
  MATH_EVAL_LOG(MATH_EVAL_SEVERITY_WARN, __VA_ARGS__)
#define MATH_EVAL_LOG_INFO(...)                                                \
  MATH_EVAL_LOG(MATH_EVAL_SEVERITY_INFO, __VA_ARGS__)
#else
#define MATH_EVAL_LOG_ERROR(...)
#define MATH_EVAL_LOG_WARN(...)
#define MATH_EVAL_LOG_INFO(...)
#endif

#ifdef __cplusplus
}
#endif

#endif // !MATH_EVAL_LOG_H
