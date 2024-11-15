## math-eval

Library for parsing basic math expressions

**math-eval** is made to closely match pythons's **eval**.

Example usage:

```c
#include "math_eval/evaluator.h"

int main (int argc, char *argv[]) {
    struct symbol_table *table = symbol_table_create();

    /* Add builtins functions such as: min, max, pow, log, sin ... */
    symbol_table_add_builtins(table);

    /* Now evaluate expression */
    double result = math_eval("5 * 10", table, NULL);

    /* If you  want to compile */
    struct math_eval_expression *expr = math_eval_compile("3 + 2^2^3 + -7^2", table, NULL);
    if (expr) {
        /* If `expr` is valid pointer then expression is successfully compiled */
        double result = math_eval_expr(expr);
        printf("%.20g\n", result);

        /* Destroy when not needed */
        math_eval_expr_destroy(expr);
    }


    struct math_eval_error error;
    symbol_table_add_variable(table, "a", 5, false);
    symbol_table_add_variable(table, "b", 9, false);

    /* Variables and functions */
    struct math_eval_expression *expr_with_variables = math_eval_compile("max(a, b) - (3 * 3) / 9", table, &error);
    if (expr_with_variables) {
        double answer = math_eval_expr(expr);
        printf("%.20g\n", answer);

        struct math_eval_variable *a = symbol_table_find_variable(table, "a");
        a->value = 0;

        answer = math_eval_expr(expr);
        printf("%.20g\n", answer);
    }

    symbol_table_destroy(table);
    math_eval_expr_destroy(expr_with_variables);

    return EXIT_SUCCESS;
}
```

### Building

---

| Option                     | Description     | Default |
| :------------------------- | :-------------- | :-----: |
| `MATH_EVAL_NOLOG`          | Disable logging |   OFF   |
| `MATH_EVAL_BUILD_EXAMPLES` | Build examples  |   OFF   |
| `MATH_EVAL_BUILD_TESTS`    | Build tests     |   OFF   |

#### Run tests

    cmake -S . -B build -G Ninja
    cmake --build build
    ctest --test-dir build/tests --verbose --output-on-failure
