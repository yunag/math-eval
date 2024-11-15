#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "math_eval/evaluator.h"
#include "math_eval/symbol_table.h"

#if 1

int app(int argc, char **argv) {
  struct symbol_table *table = symbol_table_create();

  symbol_table_add_builtins(table);

  symbol_table_add_variable(table, "a", 400, false);
  struct math_eval_variable *a = symbol_table_find_variable(table, "a");

  struct math_eval_expression *expr =
      math_eval_compile("1 / (a + 1) + 2 / (a + 2) + 3 / (a + 3)", table, NULL);

  double sum = 0;

  for (int i = 0; i < 1e8; ++i) {
    a->value = i;
    sum += math_eval_expr(expr);
  }

  printf("%f\n", sum);

  math_eval_expr_destroy(expr);
  symbol_table_destroy(table);
  return EXIT_SUCCESS;
}

#else

int app(int argc, char **argv) {
  double sum = 0;

  for (int i = 0; i < 1e8; ++i) {
    double res = (1. / (i + 1.) + 2. / (i + 2) + 3. / (i + 3));
    sum += res;
  }

  printf("%f\n", sum);

  return EXIT_SUCCESS;
}

#endif

int main(int argc, char *argv[]) { return app(argc, argv); }
