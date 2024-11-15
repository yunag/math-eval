#ifndef MATH_EVAL_CONTAINER_H
#define MATH_EVAL_CONTAINER_H

#define container_of(ptr, type, member)                                        \
  ((type *)(void *)(((char *)(ptr)) - offsetof(type, member)))

#endif /* !MATH_EVAL_CONTAINER_H */
