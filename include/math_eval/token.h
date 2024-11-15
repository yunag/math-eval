#ifndef MATH_EVAL_TOKEN_H
#define MATH_EVAL_TOKEN_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef bool (*tokenfn)(const char *, size_t *);

enum token_type {
  TOK_NUMBER = 0x1,
  TOK_PLUS = 0x2,
  TOK_MINUS = 0x4,
  TOK_IDENTIFIER = 0x8,
  TOK_OPEN_PAREN = 0x10,
  TOK_CLOSE_PAREN = 0x20,
  TOK_ASTERISK = 0x40,
  TOK_FORW_SLASH = 0x80,
  TOK_COMMA = 0x100,
  TOK_EOF = 0x200,
  TOK_CARET = 0x400,
  TOK_PERCENT = 0x800,
  TOK_INVALID = 0x1000,
  TOK_LAST_TOKEN = TOK_INVALID,
};

struct token {
  int size;
  int offset;
  enum token_type type;
};

const char *token_type_to_str(enum token_type type);
const char *token_type_to_kind(enum token_type type);

#ifdef __cplusplus
}
#endif

#endif /* !MATH_EVAL_TOKEN_H */
