#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "math_eval/evaluator.h"
#include "math_eval/log.h"
#include "math_eval/parser.h"
#include "math_eval/symbol_table.h"

static inline void indent(int n) {
  for (int i = 0; i < n; i++) {
    putchar('\t');
  }
}

#define PRINT_AST(...)                                                         \
  do {                                                                         \
    indent(depth);                                                             \
    printf(__VA_ARGS__);                                                       \
  } while (0)

static void traverse_ast(struct ast_node *ast, const char *str, int depth) {
  if (!ast) {
    return;
  }

  PRINT_AST("type: '%s'\n", ast_node_type_to_str(ast->type));
  PRINT_AST("value: '%.*s'\n", ast->size, str + ast->offset);

  switch (ast->type) {
  case AST_BINARY: {
    struct ast_node_binary *binary = ast_cast(ast, struct ast_node_binary);
    if (binary->left) {
      PRINT_AST("left: \n");
    }

    traverse_ast(binary->left, str, depth + 1);

    if (binary->right) {
      PRINT_AST("right: \n");
    }

    traverse_ast(binary->right, str, depth + 1);
    break;
  }

  case AST_CALL: {
    struct ast_node_function *fun = ast_cast(ast, struct ast_node_function);

    for (int i = 0; i < fun->args_count; ++i) {
      PRINT_AST("args[%d]: \n", i);
      traverse_ast(fun->args[i], str, depth + 1);
    }
    break;
  }

  case AST_UNARY: {
    struct ast_node_unary *unary = ast_cast(ast, struct ast_node_unary);

    PRINT_AST("arg: \n");
    traverse_ast(unary->arg, str, depth + 1);
    break;
  }

  case AST_NUMBER:
  case AST_IDENTIFIER:
    break;
  }
}

static bool check_number(const char *s) {
  if (*s == '\0' || *s == '\n') {
    return false;
  }

  if (*s == '-' || *s == '+') {
    s++;
  }

  for (; *s && *s != '\n'; ++s) {
    if (!isdigit(*s)) {
      return false;
    }
  }

  return true;
}

static int get_int(int low, int high, const char *msg) {
  char input[BUFSIZ];
  int num;

  do {
    fgets(input, sizeof(input), stdin);

    if (check_number(input) && (num = atoi(input)) >= low && num <= high) {
      break;
    }

    printf("%s\n", msg);
  } while (true);

  return num;
}

#define BUFFER_SIZE 1024

int app(int argc, char **argv) {
  struct symbol_table *table = symbol_table_create();
  if (!table) {
    MATH_EVAL_LOG_ERROR("Failed to create symbol table");
    return EXIT_FAILURE;
  }

  symbol_table_add_builtins(table);

  char buffer[BUFFER_SIZE];
  char ast_buffer[BUFFER_SIZE];

  struct ast_node *ast = NULL;

  printf("\nAvailable functions:\n"
         "\tmax(x, y)\n"
         "\tmin(x, y)\n"
         "\tlog(x)\n"
         "\tlogn(base, x)\n"
         "\tceil(x)\n"
         "\tfloor(x)\n"
         "\tabs(x)\n"
         "\tcos(x)\n"
         "\tsin(x)\n"
         "\ttan(x)\n"
         "\texp(x)\n"
         "\tround(x)\n"
         "\tpow(base, exponent)\n"
         "\tsqrt(x)\n"
         "\nAvailable constants:\n"
         "\tpi\n"
         "\te\n"
         "\nAvailable operators:\n"
         "\t'+' - sum\n"
         "\t'-' - subtract\n"
         "\t'*' - multiply\n"
         "\t'/' - divide\n"
         "\t'/' - divide\n"
         "\t'^' - exponent\n"
         "\n");

  do {
    char *f = fgets(buffer, sizeof(buffer), stdin);
    buffer[strcspn(buffer, "\r\n")] = '\0';

    if (strcmp(buffer, "q") == 0 || !f) {
      break;
    }

    if (ast && strcmp(buffer, "ast") == 0) {
      traverse_ast(ast, ast_buffer, 0);
      continue;
    }

    strcpy(ast_buffer, buffer);

    ast_destroy(ast);
    ast = ast_build(ast_buffer, NULL);

    if (!ast) {
      MATH_EVAL_LOG_ERROR("Failed to build AST: %s", ast_buffer);
      continue;
    }

    struct math_eval_expression *expr =
        math_eval_compile_ast(ast, ast_buffer, table, NULL);
    if (expr) {
      double result = math_eval_expr(expr);
      math_eval_expr_destroy(expr);
      snprintf(buffer, sizeof(buffer), "%.20g", result);

      char *dot = strchr(buffer, '.');
      if (dot) {
        char *c = dot;
        while (*c != '\0') {
          c++;
        }

        while (c != dot && *c == '0') {
          *c = '\0';
        }

        if (c == dot) {
          *c = '\0';
        }
      }

      puts(buffer);
    }
  } while (true);

  ast_destroy(ast);
  symbol_table_destroy(table);
  return EXIT_SUCCESS;
}

int main(int argc, char *argv[]) { return app(argc, argv); }
