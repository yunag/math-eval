#ifndef MATH_EVAL_TOKENIZER_H
#define MATH_EVAL_TOKENIZER_H

#include "token.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

struct tokenizer {
  int cursor;

  const char *str;
  int str_size;
};

void tokenizer_init(struct tokenizer *tok);

void tokenizer_read(struct tokenizer *tokenizer, const char *str);
struct token tokenizer_next(struct tokenizer *tokenizer);

#ifdef __cplusplus
}
#endif

#endif /* !MATH_EVAL_TOKENIZER_H */
