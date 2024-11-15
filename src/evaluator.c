#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "datastructs/memory.h"

#include "math_eval/evaluator.h"
#include "math_eval/log.h"
#include "math_eval/parser.h"
#include "math_eval/symbol_table.h"

inline enum math_eval_arithmetic_operation
ast_op_to_arithmetic_op(const char op) {
  switch (op) {
  case '+':
    return MATH_EVAL_OP_ADD;
  case '-':
    return MATH_EVAL_OP_SUB;
  case '*':
    return MATH_EVAL_OP_MUL;
  case '/':
    return MATH_EVAL_OP_DIV;
  case '%':
    return MATH_EVAL_OP_REM;
  case '^':
    return MATH_EVAL_OP_EXP;
  }

  assert(0);
  return MATH_EVAL_OP_ADD;
}

#define MATH_EVAL_BINARY_FUN                                                   \
  const struct math_eval_node_binary *binary =                                 \
      ast_cast(expr, struct math_eval_node_binary);                            \
  const struct math_eval_expression *left = binary->left;                      \
  const struct math_eval_expression *right = binary->right

static inline double
math_eval_binary_add(const struct math_eval_expression *expr) {
  MATH_EVAL_BINARY_FUN;
  return left->value(left) + right->value(right);
}

static inline double
math_eval_binary_sub(const struct math_eval_expression *expr) {
  MATH_EVAL_BINARY_FUN;
  return left->value(left) - right->value(right);
}

static inline double
math_eval_binary_mul(const struct math_eval_expression *expr) {
  MATH_EVAL_BINARY_FUN;
  return left->value(left) * right->value(right);
}

static inline double
math_eval_binary_div(const struct math_eval_expression *expr) {
  MATH_EVAL_BINARY_FUN;
  return left->value(left) / right->value(right);
}

static inline double
math_eval_binary_exp(const struct math_eval_expression *expr) {
  MATH_EVAL_BINARY_FUN;
  return pow(left->value(left), right->value(right));
}

static inline double
math_eval_binary_rem(const struct math_eval_expression *expr) {
  MATH_EVAL_BINARY_FUN;
  return fmod(left->value(left), right->value(right));
}

static inline math_eval_value_fun
math_eval_binary_value_from_op(const enum math_eval_arithmetic_operation op) {
  switch (op) {
  case MATH_EVAL_OP_ADD:
    return math_eval_binary_add;
  case MATH_EVAL_OP_SUB:
    return math_eval_binary_sub;
  case MATH_EVAL_OP_DIV:
    return math_eval_binary_div;
  case MATH_EVAL_OP_MUL:
    return math_eval_binary_mul;
  case MATH_EVAL_OP_REM:
    return math_eval_binary_rem;
  case MATH_EVAL_OP_EXP:
    return math_eval_binary_exp;
  }
}

static inline double
math_eval_number_value(const struct math_eval_expression *expr) {
  const struct math_eval_node_number *number =
      ast_cast(expr, struct math_eval_node_number);
  return number->value;
}

static inline double
math_eval_variable_value(const struct math_eval_expression *expr) {
  const struct math_eval_node_variable *var =
      ast_cast(expr, struct math_eval_node_variable);

  return *var->variable;
}

static inline double
math_eval_unary_value(const struct math_eval_expression *expr) {
  const struct math_eval_node_unary *unary =
      ast_cast(expr, struct math_eval_node_unary);

  const double arg = unary->arg->value(unary->arg);
  return unary->op == MATH_EVAL_UNARY_MINUS ? -arg : arg;
}

static inline double
math_eval_function_value(const struct math_eval_expression *expr) {
  const struct math_eval_node_function *fun =
      ast_cast(expr, struct math_eval_node_function);

  double args[fun->args_count];
  for (int i = 0; i < fun->args_count; ++i) {
    const struct math_eval_expression *arg = fun->args[i];

    args[i] = arg->value(arg);
  }

  return fun->function(args);
}

static inline double
math_eval_evaluate_binary(enum math_eval_arithmetic_operation op, double left,
                          double right) {
  switch (op) {
  case MATH_EVAL_OP_ADD:
    return left + right;
  case MATH_EVAL_OP_SUB:
    return left - right;
  case MATH_EVAL_OP_DIV:
    return left / right;
  case MATH_EVAL_OP_MUL:
    return left * right;
  case MATH_EVAL_OP_REM:
    return fmod(left, right);
  case MATH_EVAL_OP_EXP:
    return pow(left, right);
  }
}

static inline struct math_eval_expression *
math_eval_number_create(double value) {
  struct math_eval_node_number *number = yu_calloc(1, sizeof(*number));
  if (!number) {
    return NULL;
  }

  number->value = value;
  number->node.type = MATH_EVAL_NUMBER;
  number->node.value = math_eval_number_value;
  return &number->node;
}

#define EXPR_VALUE_BUFFER(buffer, ast)                                         \
  char buffer[ast->size + 1];                                                  \
  memcpy(buffer, expression + ast->offset, (size_t)ast->size);                 \
  buffer[ast->size] = '\0';

static inline void math_eval_set_error(struct math_eval_error *error,
                                       enum math_eval_error_code code,
                                       struct ast_node *ast) {
  error->code = code;
  error->offset = ast->offset;
  error->size = ast->size;
}

static struct math_eval_expression *
ast_construct_expression_tree(struct ast_node *ast, const char *expression,
                              struct symbol_table *table,
                              struct math_eval_error *error) {
  switch (ast->type) {

  case AST_NUMBER: {
    EXPR_VALUE_BUFFER(buffer, ast);
    return math_eval_number_create(atof(buffer));
  }

  case AST_BINARY: {
    struct ast_node_binary *ast_binary = ast_cast(ast, struct ast_node_binary);

    enum math_eval_arithmetic_operation op =
        ast_op_to_arithmetic_op(expression[ast->offset]);
    struct math_eval_expression *left = ast_construct_expression_tree(
        ast_binary->left, expression, table, error);
    struct math_eval_expression *right = ast_construct_expression_tree(
        ast_binary->right, expression, table, error);

    if (left->type == MATH_EVAL_NUMBER && right->type == MATH_EVAL_NUMBER) {
      /* E.g 2 + 2 = 4 or 3 * 9 = 27 */

      double result =
          math_eval_evaluate_binary(op, left->value(left), right->value(right));

      struct math_eval_expression *expr = math_eval_number_create(result);

      /* Remove branches */
      math_eval_expr_destroy(left);
      math_eval_expr_destroy(right);

      return expr;
    }

    struct math_eval_node_binary *binary = yu_calloc(1, sizeof(*binary));
    if (!binary) {
      return NULL;
    }

    binary->op = op;
    binary->left = left;
    binary->right = right;

    binary->node.type = MATH_EVAL_BINARY;
    binary->node.value = math_eval_binary_value_from_op(binary->op);

    return &binary->node;
  }

  case AST_UNARY: {
    struct ast_node_unary *ast_unary = ast_cast(ast, struct ast_node_unary);

    struct math_eval_expression *arg =
        ast_construct_expression_tree(ast_unary->arg, expression, table, error);

    if (arg->type == MATH_EVAL_NUMBER) {
      struct math_eval_node_number *number =
          ast_cast(arg, struct math_eval_node_number);

      double result = arg->value(arg);

      number->value = expression[ast->offset] == '-' ? -result : result;
      return arg;
    }

    if (arg->type == MATH_EVAL_UNARY) {
      struct math_eval_node_unary *unary =
          ast_cast(arg, struct math_eval_node_unary);

      if (expression[ast->offset] == '-') {
        unary->op = unary->op == MATH_EVAL_UNARY_PLUS ? MATH_EVAL_UNARY_MINUS
                                                      : MATH_EVAL_UNARY_PLUS;
      }

      return arg;
    }

    struct math_eval_node_unary *unary = yu_calloc(1, sizeof(*unary));
    if (!unary) {
      return NULL;
    }

    unary->arg =
        ast_construct_expression_tree(ast_unary->arg, expression, table, error);
    unary->op = expression[ast->offset] == '-' ? MATH_EVAL_UNARY_MINUS
                                               : MATH_EVAL_UNARY_PLUS;
    unary->node.type = MATH_EVAL_UNARY;
    unary->node.value = math_eval_unary_value;

    return &unary->node;
  }

  case AST_CALL: {
    EXPR_VALUE_BUFFER(buffer, ast);

    struct math_eval_function *fncall =
        symbol_table_find_function(table, buffer);
    if (!fncall) {
      math_eval_set_error(error, EVAL_ERR_NO_FUNCTION, ast);

      MATH_EVAL_LOG_ERROR("Function with name '%s' doesn't exist", buffer);
      return NULL;
    }

    struct ast_node_function *ast_fun = ast_cast(ast, struct ast_node_function);

    if (ast_fun->args_count != fncall->args_count) {
      math_eval_set_error(error, EVAL_ERR_ARGS_MISMATCH, ast);

      error->function_error.args_count_got = ast_fun->args_count;
      error->function_error.args_count_expected = fncall->args_count;

      MATH_EVAL_LOG_ERROR(
          "Function with the name '%s' expects %d arguments, but got %d",
          buffer, fncall->args_count, ast_fun->args_size);
      return NULL;
    }

    struct math_eval_expression *args[AST_CALL_MAXIMUM_NUMBER_OF_ARGUMENTS];

    bool constant_function = true;
    for (int i = 0; i < fncall->args_count; ++i) {
      struct math_eval_expression *arg = ast_construct_expression_tree(
          ast_fun->args[i], expression, table, error);

      args[i] = arg;

      if (arg->type != MATH_EVAL_NUMBER) {
        constant_function = false;
      }
    }

    if (constant_function) {
      double args_computed[AST_CALL_MAXIMUM_NUMBER_OF_ARGUMENTS];
      for (int i = 0; i < fncall->args_count; ++i) {
        struct math_eval_expression *arg = args[i];

        args_computed[i] = arg->value(arg);
        math_eval_expr_destroy(arg);
      }

      double result = fncall->function(args_computed);

      return math_eval_number_create(result);
    }

    struct math_eval_node_function *fun = yu_calloc(1, sizeof(*fun));
    if (!fun) {
      return NULL;
    }

    fun->function = fncall->function;
    fun->args_count = ast_fun->args_count;
    fun->args = yu_calloc((size_t)fun->args_count, sizeof(*fun->args));
    fun->node.type = MATH_EVAL_FUNCTION;
    fun->node.value = math_eval_function_value;

    memcpy(fun->args, args, sizeof(args[0]) * (size_t)fun->args_count);

    return &fun->node;
  }

  case AST_IDENTIFIER: {
    EXPR_VALUE_BUFFER(buffer, ast);

    struct math_eval_variable *variable =
        symbol_table_find_variable(table, buffer);
    if (!variable) {
      math_eval_set_error(error, EVAL_ERR_NO_VARIABLE, ast);

      MATH_EVAL_LOG_ERROR("Variable with name '%s' doesn't exist", buffer);
      return NULL;
    }

    if (variable->constant) {
      return math_eval_number_create(variable->value);
    }

    struct math_eval_node_variable *var = yu_calloc(1, sizeof(*var));
    if (!var) {
      return NULL;
    }

    var->variable = &variable->value;
    var->node.type = MATH_EVAl_VARIABLE;
    var->node.value = math_eval_variable_value;

    return &var->node;
  }
  }

  return NULL;
}

struct math_eval_expression *
math_eval_compile_ast(struct ast_node *ast, const char *expression,
                      struct symbol_table *table,
                      struct math_eval_error *error) {
  struct math_eval_error err;
  if (!error) {
    error = &err;
  }

  return ast_construct_expression_tree(ast, expression, table, error);
}

struct math_eval_expression *math_eval_compile(const char *expression,
                                               struct symbol_table *table,
                                               struct math_eval_error *error) {
  struct ast_error err;
  struct ast_node *ast = ast_build(expression, &err);
  if (!ast) {
    error->offset = err.offset;
    error->code = EVAL_ERR_PARSE;
    return NULL;
  }

  struct math_eval_expression *expr =
      math_eval_compile_ast(ast, expression, table, error);

  ast_destroy(ast);
  return expr;
}

void math_eval_expr_destroy(struct math_eval_expression *expression) {
  if (!expression) {
    return;
  }

  switch (expression->type) {
  case MATH_EVAL_NUMBER: {
    struct math_eval_node_number *number =
        ast_cast(expression, struct math_eval_node_number);
    yu_free(number);
    break;
  }

  case MATH_EVAL_FUNCTION: {
    struct math_eval_node_function *fun =
        ast_cast(expression, struct math_eval_node_function);
    for (int i = 0; i < fun->args_count; ++i) {
      math_eval_expr_destroy(fun->args[i]);
    }

    yu_free(fun->args);
    yu_free(fun);
    break;
  }

  case MATH_EVAL_UNARY: {
    struct math_eval_node_unary *unary =
        ast_cast(expression, struct math_eval_node_unary);
    math_eval_expr_destroy(unary->arg);

    yu_free(unary);
    break;
  }

  case MATH_EVAL_BINARY: {
    struct math_eval_node_binary *binary =
        ast_cast(expression, struct math_eval_node_binary);
    math_eval_expr_destroy(binary->left);
    math_eval_expr_destroy(binary->right);

    yu_free(binary);
    break;
  }

  case MATH_EVAl_VARIABLE: {
    struct math_eval_node_variable *var =
        ast_cast(expression, struct math_eval_node_variable);
    yu_free(var);
    break;
  }
  }
}

double math_eval(const char *expression, struct symbol_table *table,
                 struct math_eval_error *error) {
  struct math_eval_expression *expr =
      math_eval_compile(expression, table, error);

  if (expr) {
    double result = math_eval_expr(expr);
    math_eval_expr_destroy(expr);

    return result;
  }

  return NAN;
}

void math_eval_init(struct math_eval_allocator *allocator) {
  assert(allocator != NULL);

  if (allocator) {
    yu_set_allocator(&(struct yu_allocator){
        .allocate = allocator->allocate,
        .deallocate = allocator->deallocate,
        .reallocate = yu_default_reallocate,
        .user_data = allocator->user_data,
    });
  }
}
