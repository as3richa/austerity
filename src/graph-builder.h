#ifndef GRAPH_BUILDER_H
#define GRAPH_BUILDER_H

#include "common.h"
#include "dsl.h"

void record_einval(graph_builder_t *g, const char *api_fn_name, const char *english);

#define NULL_CHECK(g, ident, ret) INVAL_CHECK(g, (ident) == NULL, #ident "is NULL", ret)

#define INVAL_CHECK(g, cond, message, ret)                                                         \
  do {                                                                                             \
    if (cond) {                                                                                    \
      record_einval((g), __func__, message);                                                       \
      return (ret);                                                                                \
    }                                                                                              \
  } while (0)

void *ialloc(graph_builder_t *g, size_t size, const char *api_fn_name);
void *irealloc(graph_builder_t *g,
               void *ptr,
               size_t elem_size,
               size_t size,
               size_t prev,
               const char *api_fn_name);
void ifree(graph_builder_t *g, void *ptr);

char *copy_buffer(graph_builder_t *g, const char *buffer, size_t size, const char *api_fn_name);

char *copy_str(graph_builder_t *g, const char *str, const char *api_fn_name);

dsl_state_t *graph_builder_dsl(graph_builder_t *g);

environment_t *graph_builder_default_env(graph_builder_t *g);

#endif
