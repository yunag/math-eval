#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "datastructs/functions.h"
#include "datastructs/hash_table.h"
#include "datastructs/memory.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include "math_eval/symbol_table.h"

struct function_call_hash {
  char *str;
  struct math_eval_function fc;

  struct hash_entry hh;
};

struct variable_hash {
  char *str;
  struct math_eval_variable value;

  struct hash_entry hh;
};

size_t hash_function_call(const struct hash_entry *entry) {
  struct function_call_hash *hash =
      htable_entry(entry, struct function_call_hash, hh);
  return yu_hash_str(hash->str);
}

bool equal_function_call(const struct hash_entry *lhs,
                         const struct hash_entry *rhs) {
  struct function_call_hash *a =
      htable_entry(lhs, struct function_call_hash, hh);
  struct function_call_hash *b =
      htable_entry(rhs, struct function_call_hash, hh);
  return strcmp(a->str, b->str) == 0;
}

void destroy_variable(struct variable_hash *variable) {
  if (variable) {
    yu_free(variable->str);
    yu_free(variable);
  }
}

void destroy_function_call(struct function_call_hash *fc) {
  if (fc) {
    yu_free(fc->str);
    yu_free(fc);
  }
}

size_t hash_variable(const struct hash_entry *entry) {
  struct variable_hash *hash = htable_entry(entry, struct variable_hash, hh);
  return yu_hash_str(hash->str);
}

bool equal_variable(const struct hash_entry *lhs,
                    const struct hash_entry *rhs) {
  struct variable_hash *a = htable_entry(lhs, struct variable_hash, hh);
  struct variable_hash *b = htable_entry(rhs, struct variable_hash, hh);
  return strcmp(a->str, b->str) == 0;
}

struct symbol_table *symbol_table_create(void) {
  struct symbol_table *table = yu_calloc(1, sizeof(*table));
  if (!table) {
    return NULL;
  }

  table->functions = htable_create(10, hash_function_call, equal_function_call);
  table->variables = htable_create(10, hash_variable, equal_variable);

  if (!table->functions || !table->variables) {
    symbol_table_destroy(table);
    return NULL;
  }

  return table;
}

void symbol_table_destroy(struct symbol_table *table) {
  if (table) {
    struct variable_hash *cur, *n;
    struct function_call_hash *fcur, *fn;

    htable_for_each_temp(table->variables, cur, n, hh) {
      destroy_variable(cur);
    }
    htable_for_each_temp(table->functions, fcur, fn, hh) {
      destroy_function_call(fcur);
    }

    htable_destroy(table->variables, NULL);
    htable_destroy(table->functions, NULL);
    yu_free(table);
  }
}

struct math_eval_variable *
symbol_table_find_variable(struct symbol_table *table, const char *key) {
  assert(table != NULL);

  struct variable_hash query;
  query.str = (char *)key;

  struct hash_entry *entry = htable_lookup(table->variables, &query.hh);
  if (!entry) {
    return NULL;
  }

  struct variable_hash *variable =
      htable_entry(entry, struct variable_hash, hh);

  return &variable->value;
}

struct math_eval_function *
symbol_table_find_function(struct symbol_table *table, const char *key) {
  assert(table != NULL);
  assert(key != NULL);

  struct function_call_hash query;
  query.str = (char *)key;

  struct hash_entry *entry = htable_lookup(table->functions, &query.hh);
  if (!entry) {
    return NULL;
  }

  struct function_call_hash *fun =
      htable_entry(entry, struct function_call_hash, hh);

  return &fun->fc;
}

bool symbol_table_add_function(struct symbol_table *table, const char *key,
                               struct math_eval_function fc) {
  assert(table != NULL);
  assert(key != NULL);

  struct function_call_hash *entry = yu_calloc(1, sizeof(*entry));
  entry->str = yu_dup_str(key);
  entry->fc = fc;

  struct hash_entry *replaced = NULL;
  bool ok = htable_replace(table->functions, &entry->hh, &replaced);

  if (replaced) {
    destroy_function_call(
        htable_entry(replaced, struct function_call_hash, hh));
  }

  return ok;
}

bool symbol_table_add_variable(struct symbol_table *table, const char *key,
                               double var, bool constant) {
  assert(table != NULL);
  assert(key != NULL);

  struct variable_hash *entry = yu_calloc(1, sizeof(*entry));
  entry->str = yu_dup_str(key);
  entry->value.constant = constant;
  entry->value.value = var;

  struct hash_entry *replaced = NULL;
  bool ok = htable_replace(table->variables, &entry->hh, &replaced);

  if (replaced) {
    destroy_variable(htable_entry(replaced, struct variable_hash, hh));
  }

  return ok;
}

static size_t ncr(int n, int r) {
  if (r > n) {
    return 0;
  }
  if (r * 2 > n) {
    r = n - r;
  }
  if (r == 0) {
    return 1;
  }

  size_t result = (size_t)n;
  for (int i = 2; i <= r; ++i) {
    result *= (size_t)(n - i + 1);
    result /= (size_t)i;
  }
  return result;
}

static double min_variadic(double *params) {
  return fmin(params[0], params[1]);
}

static double max_variadic(double *params) {
  return fmax(params[0], params[1]);
}

static double log_variadic(double *params) { return log(params[0]); }

static double logn_variadic(double *params) {
  return log(params[1]) / log(params[0]);
}

static double ceil_variadic(double *params) { return ceil(params[0]); }

static double floor_variadic(double *params) { return floor(params[0]); }

static double abs_variadic(double *params) { return fabs(params[0]); }

static double cos_variadic(double *params) { return cos(params[0]); }

static double sin_variadic(double *params) { return sin(params[0]); }

static double tan_variadic(double *params) { return tan(params[0]); }

static double sqrt_variadic(double *params) { return sqrt(params[0]); }

static double exp_variadic(double *params) { return exp(params[0]); }

static double round_variadic(double *params) { return round(params[0]); }

static double pow_variadic(double *params) { return pow(params[0], params[1]); }

static double ncr_variadic(double *params) {
  return (double)ncr((int)params[0], (int)params[1]);
}

void symbol_table_add_builtins(struct symbol_table *table) {
  const struct builtin_function {
    const char *name;

    double (*function)(double *);
    int args_count;
  } builtins_functions[] = {
      {"min", min_variadic, 2},     {"max", max_variadic, 2},
      {"logn", logn_variadic, 2},   {"log", log_variadic, 1},
      {"ceil", ceil_variadic, 1},   {"floor", floor_variadic, 1},
      {"abs", abs_variadic, 1},     {"cos", cos_variadic, 1},
      {"sin", sin_variadic, 1},     {"exp", exp_variadic, 1},
      {"round", round_variadic, 1}, {"pow", pow_variadic, 2},
      {"sqrt", sqrt_variadic, 1},   {"tan", tan_variadic, 1},
      {"ncr", ncr_variadic, 2}};

  const int builtins_functions_count =
      sizeof(builtins_functions) / sizeof(builtins_functions[0]);
  for (int i = 0; i < builtins_functions_count; ++i) {
    const struct builtin_function *builtin = &builtins_functions[i];

    struct math_eval_function fun;
    fun.function = builtin->function;
    fun.args_count = builtin->args_count;

    symbol_table_add_function(table, builtin->name, fun);
  }

  symbol_table_add_variable(table, "pi", M_PI, true);
  symbol_table_add_variable(table, "e", M_E, true);
  symbol_table_add_variable(table, "pi_2", M_PI_2, true);
  symbol_table_add_variable(table, "pi_4", M_PI_4, true);
}
