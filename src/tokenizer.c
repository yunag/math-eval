#include <assert.h>
#include <ctype.h>
#include <string.h>

#include "math_eval/log.h"
#include "math_eval/token.h"
#include "math_eval/tokenizer.h"

void tokenizer_init(struct tokenizer *tok) {
  tok->str = NULL;
  tok->str_size = 0;
  tok->cursor = 0;
}

void tokenizer_read(struct tokenizer *tok, const char *str) {
  assert(tok != NULL);
  assert(str != NULL);

  tok->str = str;
  tok->str_size = (int)strlen(str);
  tok->cursor = 0;
}

int extract_number(const char *s) {
  bool saw_exp = false;
  bool saw_dot = false;
  bool saw_dig = false;

  const char *start = s;

  for (; *s != '\0'; ++s) {
    if (isdigit(*s)) {
      saw_dig = true;
    } else if (*s == '.') {
      if (saw_dot || saw_exp) {
        return 0;
      }

      saw_dot = true;
    } else if (*s == 'e' || *s == 'E') {
      /* If we already saw `e` or not saw `0-9` it's not a valid number */
      if (!saw_dig || saw_exp) {
        return 0;
      }

      /* Skip `e` sign */
      if (s[1] == '-' || s[1] == '+') {
        s++;
      }

      saw_dig = false;
      saw_exp = true;
    } else {
      break;
    }
  }

  return saw_dig ? (int)(s - start) : 0;
}

struct token tokenizer_next(struct tokenizer *tok) {
  assert(tok != NULL);
  assert(tok->str != NULL);

  /* Skip white spaces */
  while (isspace(tok->str[tok->cursor])) {
    tok->cursor++;
  }

  if (tok->cursor >= tok->str_size) {
    return (struct token){.size = 0, .offset = 0, .type = TOK_EOF};
  }

  struct token t;
  t.offset = tok->cursor;
  t.type = TOK_INVALID;

  switch (tok->str[tok->cursor]) {
  case '0':
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
  case '8':
  case '9':
  case '.': {
    t.size = extract_number(&tok->str[tok->cursor]);

    if (t.size > 0) {
      t.type = TOK_NUMBER;
    }
    break;
  }

  case '-': {
    t.type = TOK_MINUS;
    t.size = 1;
    break;
  }

  case '+': {
    t.type = TOK_PLUS;
    t.size = 1;
    break;
  }

  case '*': {
    t.type = TOK_ASTERISK;
    t.size = 1;
    break;
  }

  case '/': {
    t.type = TOK_FORW_SLASH;
    t.size = 1;
    break;
  }

  case '%': {
    t.type = TOK_PERCENT;
    t.size = 1;
    break;
  }

  case '^': {
    t.type = TOK_CARET;
    t.size = 1;
    break;
  }

  case ',': {
    t.type = TOK_COMMA;
    t.size = 1;
    break;
  }

  case '(': {
    t.type = TOK_OPEN_PAREN;
    t.size = 1;
    break;
  }

  case ')': {
    t.type = TOK_CLOSE_PAREN;
    t.size = 1;
    break;
  }

  default: {
    /* Check if identifier */
    const char *s = &tok->str[tok->cursor];

    for (; *s != '\0' && (isalpha(*s) || *s == '_' || isdigit(*s)); ++s) {
    }

    t.size = (int)(s - &tok->str[tok->cursor]);

    if (t.size > 0) {
      t.type = TOK_IDENTIFIER;
      break;
    }

    /* Could not extract any tokens */
    MATH_EVAL_LOG_ERROR("Invalid syntax: %s", &tok->str[tok->cursor]);
  }
  }

  tok->cursor += t.size;
  return t;
}
