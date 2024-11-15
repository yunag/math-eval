#ifndef MATH_EVAL_SYMBOL_TABLE_H
#define MATH_EVAL_SYMBOL_TABLE_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef double (*math_fn)(double *);

struct hash_table;

struct math_eval_function {
  math_fn function;
  int args_count;
};

struct math_eval_variable {
  double value;
  bool constant;
};

struct symbol_table {
  struct hash_table *functions;
  struct hash_table *variables;
};

struct symbol_table *symbol_table_create(void);
void symbol_table_destroy(struct symbol_table *table);

void symbol_table_add_builtins(struct symbol_table *table);

struct math_eval_variable *
symbol_table_find_variable(struct symbol_table *table, const char *key);
struct math_eval_function *
symbol_table_find_function(struct symbol_table *table, const char *key);
bool symbol_table_add_function(struct symbol_table *table, const char *key,
                               struct math_eval_function fc);
bool symbol_table_add_variable(struct symbol_table *table, const char *key,
                               double var, bool constant);
#ifdef __cplusplus
}
#endif

#endif /* !MATH_EVAL_SYMBOL_TABLE_H */
