#ifndef ERRORS_H
#define ERRORS_H

#include "common.h"

struct errors;

void initialize_errors(struct errors *err);

void record_einval(graph_builder_t *g, const char *call, const char *english);
void record_alloc_failure(graph_builder_t *g, const char *call);

#define NULL_CHECK(g, ident, ret) INVAL_CHECK(g, (ident) == NULL, #ident "is NULL", ret)

#define INVAL_CHECK(g, cond, message, ret)                                                         \
  do {                                                                                             \
    if (cond) {                                                                                    \
      record_einval((g), __func__, message);                                                       \
      return (ret);                                                                                \
    }                                                                                              \
  } while (0)

#endif
