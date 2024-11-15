#include <stdlib.h>
#include <string.h>

#include "math_eval/evaluator.h"
#include "math_eval/log.h"
#include "math_eval/parser.h"
#include "math_eval/symbol_table.h"

int main(int argc, char *argv[]) {
  struct symbol_table *table = symbol_table_create();
  if (!table) {
    MATH_EVAL_LOG_ERROR("Failed to create symbol table");
    return EXIT_FAILURE;
  }

  const char *variables[] = {"a", "b", "c", "x", "y", "z", "w"};
  int variables_count = sizeof(variables) / sizeof(variables[0]);
  if (variables_count != argc - 1) {
    return EXIT_FAILURE;
  }

  for (int i = 0; i < variables_count; ++i) {
    symbol_table_add_variable(table, variables[i], atof(argv[i + 1]), true);
  }

  symbol_table_add_builtins(table);

  char buffer[BUFSIZ];

  struct ast_node *ast = NULL;

  while (fgets(buffer, sizeof(buffer), stdin)) {
    buffer[strcspn(buffer, "\r\n")] = '\0';

    struct math_eval_expression *expr = math_eval_compile(buffer, table, NULL);
    if (expr) {
      double result = math_eval_expr(expr);
      math_eval_expr_destroy(expr);

      printf("%.20g\n", result);
    } else {
      printf("[FAIL] %s\n", buffer);
    }

    fflush(stdout);
  }

  ast_destroy(ast);
  symbol_table_destroy(table);
  return EXIT_SUCCESS;
}
