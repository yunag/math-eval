#ifndef MATH_EVAL_PARSER_H
#define MATH_EVAL_PARSER_H

#include <stdbool.h>
#include <stddef.h>

#include "container.h"
#include "tokenizer.h"

#ifdef __cplusplus
extern "C" {
#endif

#define AST_CALL_MAXIMUM_NUMBER_OF_ARGUMENTS 256

enum ast_node_type {
  AST_NUMBER,
  AST_BINARY,
  AST_UNARY,
  AST_CALL,
  AST_IDENTIFIER,
};

enum ast_error_codes {
  AST_NO_ERROR = 0x0,
  AST_ERR_EXPECTED_EXPRESSION = 0x20,
  AST_ERR_UNEXPECTED_EOF = 0x40,
  AST_ERR_UNEXPECTED_TOKEN = 0x80,
  AST_ERR_FATAL = 0xfffffff,
};

struct ast_node {
  enum ast_node_type type; /* AST node type */

  int offset; /* Offset of a token */
  int size;   /* Length of a token */
};

struct ast_node_binary {
  struct ast_node node;

  struct ast_node *left;  /* Pointer to the left child */
  struct ast_node *right; /* Pointer to the right child */
};

struct ast_node_function {
  struct ast_node node;

  struct ast_node **args; /* Function arguments */
  int args_count;         /* Count of arguments in a function */
};

struct ast_node_variable {
  struct ast_node node;
};

struct ast_node_unary {
  struct ast_node node;

  struct ast_node *arg;
};

struct ast_error {
  int offset;
  enum ast_error_codes codes;
};

struct parser {
  struct tokenizer tokenizer;
  struct token lookahead;

  enum ast_error_codes error_codes;
  int error_offset;
};

#define ast_cast(ast, type) container_of(ast, type, node)

void ast_node_destroy(struct ast_node *node);
void ast_destroy(struct ast_node *ast);
struct ast_node *ast_build(const char *str, struct ast_error *error);

void parser_init(struct parser *parser);
struct ast_node *parser_read(struct parser *parser, const char *str);

const char *ast_node_type_to_str(enum ast_node_type type);

#ifdef __cplusplus
}
#endif

#endif /* !MATH_EVAL_PARSER_H */
