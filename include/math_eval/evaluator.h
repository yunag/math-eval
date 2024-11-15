#ifndef MATH_EVAL_EVALUATOR_H
#define MATH_EVAL_EVALUATOR_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct ast_node;
struct symbol_table;

enum math_eval_error_code {
  EVAL_NO_ERROR = 0,
  EVAL_ERR_NO_FUNCTION = 0x1,
  EVAL_ERR_NO_VARIABLE = 0x2,
  EVAL_ERR_ARGS_MISMATCH = 0x4,
  EVAL_ERR_PARSE = 0x20,
};

struct math_eval_error {
  enum math_eval_error_code code;

  int offset;
  int size;

  union {
    struct {
      int args_count_expected;
      int args_count_got;
    } function_error;
  };
};

struct math_eval_allocator {
  void *(*allocate)(size_t, void *);
  void (*deallocate)(void *, void *);

  void *user_data;
};

enum math_eval_node_type {
  MATH_EVAL_NUMBER = 0,
  MATH_EVAL_FUNCTION,
  MATH_EVAL_UNARY,
  MATH_EVAL_BINARY,
  MATH_EVAl_VARIABLE,
};

struct math_eval_expression {
  enum math_eval_node_type type;

  double (*value)(const struct math_eval_expression *);
};

typedef double (*math_eval_value_fun)(const struct math_eval_expression *);

struct math_eval_node_variable {
  struct math_eval_expression node;

  const double *variable;
};

struct math_eval_node_function {
  struct math_eval_expression node;

  double (*function)(double *);

  struct math_eval_expression **args;
  int args_count;
};

struct math_eval_node_number {
  struct math_eval_expression node;

  double value;
};

struct math_eval_node_unary {
  struct math_eval_expression node;

  struct math_eval_expression *arg;
  enum math_eval_unary_op {
    MATH_EVAL_UNARY_MINUS,
    MATH_EVAL_UNARY_PLUS,
  } op;
};

struct math_eval_node_binary {
  struct math_eval_expression node;

  struct math_eval_expression *left;
  struct math_eval_expression *right;

  enum math_eval_arithmetic_operation {
    MATH_EVAL_OP_ADD,
    MATH_EVAL_OP_SUB,
    MATH_EVAL_OP_DIV,
    MATH_EVAL_OP_MUL,
    MATH_EVAL_OP_REM,
    MATH_EVAL_OP_EXP,
  } op;
};

double math_eval(const char *expression, struct symbol_table *table,
                 struct math_eval_error *error);

static inline double math_eval_expr(const struct math_eval_expression *expr) {
  return expr->value(expr);
}

struct math_eval_expression *math_eval_compile(const char *expression,
                                               struct symbol_table *table,
                                               struct math_eval_error *error);
struct math_eval_expression *
math_eval_compile_ast(struct ast_node *ast, const char *expression,
                      struct symbol_table *table,
                      struct math_eval_error *error);
void math_eval_expr_destroy(struct math_eval_expression *expression);

void math_eval_init(struct math_eval_allocator *allocator);

#ifdef __cplusplus
}
#endif

#endif /* !MATH_EVAL_EVALUATOR_H */
