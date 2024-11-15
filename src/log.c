#include <stdarg.h>
#include <stdio.h>

#include "math_eval/log.h"

static void default_log_handler(enum math_eval_log_severity severity,
                                const char *msg) {
  (void)severity;

  printf("%s\n", msg);
}

static math_eval_message_handler log_handler = default_log_handler;

void math_eval_install_message_handler(math_eval_message_handler handler) {
  log_handler = handler;
}

void math_eval_log_message(enum math_eval_log_severity severity,
                           const char *msg) {
  if (log_handler) {
    log_handler(severity, msg);
  }
}
