#include <assert.h>

#include "math_eval/token.h"

const char *token_type_to_str(enum token_type type) {
  switch (type) {
  case TOK_NUMBER:
    return "number";
  case TOK_PLUS:
    return "plus";
  case TOK_MINUS:
    return "minus";
  case TOK_IDENTIFIER:
    return "identifier";
  case TOK_OPEN_PAREN:
    return "open parenthesis";
  case TOK_CLOSE_PAREN:
    return "close parenthesis";
  case TOK_ASTERISK:
    return "asterisk";
  case TOK_FORW_SLASH:
    return "forward slash";
  case TOK_COMMA:
    return "comma";
  case TOK_CARET:
    return "caret";
  case TOK_EOF:
    return "EOF";
  case TOK_INVALID:
    return "invalid";
  case TOK_PERCENT:
    return "percent";
  }
  return "unknown type";
}

const char *token_type_to_kind(enum token_type type) {
  switch (type) {
  case TOK_EOF:
  case TOK_IDENTIFIER:
  case TOK_NUMBER:
  case TOK_INVALID:
    return token_type_to_str(type);
  case TOK_PLUS:
    return "+";
  case TOK_MINUS:
    return "-";
  case TOK_OPEN_PAREN:
    return "(";
  case TOK_CLOSE_PAREN:
    return ")";
  case TOK_ASTERISK:
    return "*";
  case TOK_FORW_SLASH:
    return "/";
  case TOK_COMMA:
    return ",";
  case TOK_CARET:
    return "^";
  case TOK_PERCENT:
    return "%";
  }
  return "unknown type";
}
