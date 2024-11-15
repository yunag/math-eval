#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "datastructs/memory.h"

#include "math_eval/log.h"
#include "math_eval/parser.h"
#include "math_eval/token.h"
#include "math_eval/tokenizer.h"

/*
 * Backus-Naur form
 *
 * EXPRESSION
 *     : ADDITION
 *     ;
 *
 * ADDITION
 *     : ADDITION ('+' | '-') MULTIPLICATION
 *     | MULTIPLICATION
 *     ;
 *
 * MULTIPLICATION
 *     : MULTIPLICATION ('*' | '/' | '%') CALL
 *     | CALL
 *     ;
 *
 * UNARY
 *     : ('-' | '+') UNARY
 *     | POWER
 *     ;
 *
 * POWER
 *     : POWER (^) UNARY
 *     | CALL
 *     ;
 *
 * CALL
 *     : identifier '(' ( EXPRESSION (',' EXPRESSION)* )? ')'
 *     | BASIC
 *     ;
 *
 * BASIC
 *     : number
 *     | identifier
 *     | '(' EXPRESSION ')'
 *     ;
 *
 */

static struct token parser_eat(struct parser *parser, int token_types);
static bool parser_token_is(struct parser *parser, int token_types);
static struct ast_node *parser_parse_expression(struct parser *parser);
static struct ast_node *parser_parse_addition(struct parser *parser);
static struct ast_node *parser_parse_multiplication(struct parser *parser);
static struct ast_node *parser_parse_power(struct parser *parser);
static struct ast_node *parser_parse_unary(struct parser *parser);
static struct ast_node *parser_parse_call(struct parser *parser);
static struct ast_node *parser_parse_basic(struct parser *parser);

struct ast_node *ast_node_init(struct ast_node *node, enum ast_node_type type,
                               int offset, int size) {
  assert(node != NULL);

  node->type = type;
  node->offset = offset;
  node->size = size;

  return node;
}

struct ast_node *ast_node_create(enum ast_node_type type, int offset,
                                 int size) {
  struct ast_node *ast_node = yu_calloc(1, sizeof(*ast_node));
  if (!ast_node) {
    return NULL;
  }

  return ast_node_init(ast_node, type, offset, size);
}

struct ast_node *ast_node_create_binary(int offset, int size,
                                        struct ast_node *left,
                                        struct ast_node *right) {
  struct ast_node_binary *binary = yu_calloc(1, sizeof(*binary));
  if (!binary) {
    return NULL;
  }

  binary->left = left;
  binary->right = right;

  return ast_node_init(&binary->node, AST_BINARY, offset, size);
}

struct ast_node *ast_node_function_create(int offset, int size) {
  struct ast_node_function *fun = yu_calloc(1, sizeof(*fun));
  if (!fun) {
    return NULL;
  }

  fun->args = NULL;
  fun->args_count = 0;

  return ast_node_init(&fun->node, AST_CALL, offset, size);
}

struct ast_node *ast_node_unary_create(int offset, int size,
                                       struct ast_node *arg) {
  struct ast_node_unary *unary = yu_calloc(1, sizeof(*unary));
  if (!unary) {
    return NULL;
  }

  unary->arg = arg;
  return ast_node_init(&unary->node, AST_UNARY, offset, size);
}

struct ast_node *ast_build(const char *str, struct ast_error *error) {
  struct parser parser;
  parser_init(&parser);

  struct ast_node *ast = parser_read(&parser, str);
  if (!ast || parser.error_codes != AST_NO_ERROR) {
    error->codes = parser.error_codes;
    error->offset = parser.error_offset;

    ast_destroy(ast);
    return NULL;
  }

  return ast;
}

void ast_node_destroy(struct ast_node *node) {
  if (node) {
    if (node->type == AST_CALL) {
      struct ast_node_function *fun = ast_cast(node, struct ast_node_function);
      for (int i = 0; i < fun->args_count; ++i) {
        ast_node_destroy(fun->args[i]);
      }

      yu_free(fun->args);
      yu_free(fun);

    } else if (node->type == AST_UNARY) {
      struct ast_node_unary *unary = ast_cast(node, struct ast_node_unary);
      ast_node_destroy(unary->arg);
      yu_free(unary);

    } else if (node->type == AST_BINARY) {
      struct ast_node_binary *binary = ast_cast(node, struct ast_node_binary);

      ast_node_destroy(binary->left);
      ast_node_destroy(binary->right);

      yu_free(binary);
    } else {
      yu_free(node);
    }
  }
}

void ast_destroy(struct ast_node *ast) {
  if (ast) {
    ast_node_destroy(ast);
  }
}

void parser_init(struct parser *parser) {
  tokenizer_init(&parser->tokenizer);

  parser->error_codes = AST_NO_ERROR;
  parser->error_offset = 0;
}

void parser_destroy(struct parser *parser) { yu_free(parser); }

struct ast_node *parser_read(struct parser *parser, const char *str) {
  assert(parser != NULL);

  tokenizer_read(&parser->tokenizer, str);
  parser->error_codes = AST_NO_ERROR;
  parser->lookahead = tokenizer_next(&parser->tokenizer);

  struct ast_node *expression = parser_parse_expression(parser);
  parser_eat(parser, TOK_EOF);

  return expression;
}

const char *ast_node_type_to_str(enum ast_node_type type) {
  switch (type) {
  case AST_NUMBER:
    return "number";
  case AST_BINARY:
    return "binary";
  case AST_UNARY:
    return "unary";
  case AST_CALL:
    return "call";
  case AST_IDENTIFIER:
    return "identifier";
  }

  return "invalid type";
}

static void token_types_to_kinds(int token_types, char *buffer, int bufsize) {
  bufsize -= 1;
  *buffer = '\0';

  enum token_type type = TOK_LAST_TOKEN;

  for (; type && bufsize > 0; type >>= 1) {
    if (token_types & type) {
      const char *kind = token_type_to_kind(type);

      int cx = snprintf(buffer, (size_t)bufsize, "%s", kind);
      if (cx < 0) {
        break;
      }

      bufsize -= cx;
      buffer += cx;
    }
  }

  *buffer = '\0';
}

static struct token parser_eat(struct parser *parser, int token_types) {
  assert(parser != NULL);

  struct token token = parser->lookahead;

  if (!parser_token_is(parser, token_types)) {
    char buffer[256];
    token_types_to_kinds(token_types, buffer, sizeof(buffer));
    parser->error_codes |= AST_ERR_UNEXPECTED_TOKEN;
    parser->error_offset = parser->tokenizer.cursor;

    MATH_EVAL_LOG_ERROR("Expected '%s', but got '%s'", buffer,
                        token_type_to_kind(token.type));
    return token;
  }
  parser->lookahead = tokenizer_next(&parser->tokenizer);
  return token;
}

static bool parser_token_is(struct parser *parser, int token_types) {
  return token_types & parser->lookahead.type;
}

static struct ast_node *parser_parse_expression(struct parser *parser) {
  return parser_parse_addition(parser);
}

static struct ast_node *parser_parse_addition(struct parser *parser) {
  struct ast_node *left = parser_parse_multiplication(parser);

  while (parser_token_is(parser, TOK_PLUS | TOK_MINUS)) {
    struct token t = parser_eat(parser, TOK_PLUS | TOK_MINUS);
    left = ast_node_create_binary(t.offset, t.size, left,
                                  parser_parse_multiplication(parser));
  }
  return left;
}

static struct ast_node *parser_parse_multiplication(struct parser *parser) {
  struct ast_node *left = parser_parse_unary(parser);

  while (parser_token_is(parser, TOK_ASTERISK | TOK_FORW_SLASH | TOK_PERCENT)) {
    struct token t =
        parser_eat(parser, TOK_ASTERISK | TOK_FORW_SLASH | TOK_PERCENT);
    left = ast_node_create_binary(t.offset, t.size, left,
                                  parser_parse_unary(parser));
  }
  return left;
}

static struct ast_node *parser_parse_unary(struct parser *parser) {
  if (parser_token_is(parser, TOK_MINUS | TOK_PLUS)) {
    struct token t = parser_eat(parser, TOK_MINUS | TOK_PLUS);

    return ast_node_unary_create(t.offset, t.size, parser_parse_unary(parser));
  }

  return parser_parse_power(parser);
}

static struct ast_node *parser_parse_power(struct parser *parser) {
  struct ast_node *left = parser_parse_call(parser);

  if (parser_token_is(parser, TOK_CARET)) {
    struct token t = parser_eat(parser, TOK_CARET);

    left = ast_node_create_binary(t.offset, t.size, left,
                                  parser_parse_unary(parser));
  }
  return left;
}

static struct ast_node *parser_parse_call(struct parser *parser) {
  struct ast_node *maybe_callee = parser_parse_basic(parser);

  /* Maybe calle might be null */
  if (maybe_callee && maybe_callee->type == AST_IDENTIFIER &&
      parser_token_is(parser, TOK_OPEN_PAREN)) {
    parser_eat(parser, TOK_OPEN_PAREN);

    struct ast_node *callee =
        ast_node_function_create(maybe_callee->offset, maybe_callee->size);
    ast_node_destroy(maybe_callee);

    /* Function with zero arguments */
    if (parser_token_is(parser, TOK_CLOSE_PAREN)) {
      parser_eat(parser, TOK_CLOSE_PAREN);
      return callee;
    }

    struct ast_node_function *fun = ast_cast(callee, struct ast_node_function);
    fun->args =
        yu_calloc(AST_CALL_MAXIMUM_NUMBER_OF_ARGUMENTS, sizeof(*fun->args));
    fun->args_count = 1;

    fun->args[0] = parser_parse_expression(parser);
    while (parser_token_is(parser, TOK_COMMA) &&
           fun->args_count < AST_CALL_MAXIMUM_NUMBER_OF_ARGUMENTS) {
      parser_eat(parser, TOK_COMMA);
      fun->args[fun->args_count++] = parser_parse_expression(parser);
    }

    parser_eat(parser, TOK_CLOSE_PAREN);

    return callee;
  }

  return maybe_callee;
}

static struct ast_node *parser_parse_basic(struct parser *parser) {
  if (parser_token_is(parser, TOK_OPEN_PAREN)) {
    parser_eat(parser, TOK_OPEN_PAREN);
    struct ast_node *expr = parser_parse_expression(parser);
    parser_eat(parser, TOK_CLOSE_PAREN);
    return expr;
  }

  if (parser_token_is(parser, TOK_NUMBER)) {
    struct token t = parser_eat(parser, TOK_NUMBER);
    return ast_node_create(AST_NUMBER, t.offset, t.size);
  }

  if (parser_token_is(parser, TOK_IDENTIFIER)) {
    struct token t = parser_eat(parser, TOK_IDENTIFIER);
    return ast_node_create(AST_IDENTIFIER, t.offset, t.size);
  }

  parser->error_codes |= AST_ERR_EXPECTED_EXPRESSION;
  parser->error_offset = parser->tokenizer.cursor;
  MATH_EVAL_LOG_ERROR("Expected expression but got: '%s'.",
                      token_type_to_kind(parser->lookahead.type));
  return NULL;
}
