#ifndef GRAPH_BUILDER_H
#define GRAPH_BUILDER_H

#include "allocator.h"
#include "common.h"
#include "errors.h"
#include "graph.h"

struct austerity_graph_builder {
  environment_t *default_env;
  struct graph gr;
  errors_t errors;
  allocator_t a;
};

#define NULL_CHECK(g, ident, ret, call)                                                            \
  INVAL_CHECK((g), (ident) == NULL, #ident "is NULL", ret, call)

#define INVAL_CHECK(g, cond, message, ret, call)                                                   \
  do {                                                                                             \
    if (cond) {                                                                                    \
      record_einval(&g->errors, call, message);                                                    \
      return (ret);                                                                                \
    }                                                                                              \
  } while (0)

#endif
