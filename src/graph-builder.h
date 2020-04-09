#ifndef GRAPH_BUILDER_H
#define GRAPH_BUILDER_H

#include "common.h"
#include "dsl.h"

struct austerity_graph_builder {
  environment_t *default_env;

  dsl_state_t dsl;

  struct errors {
    struct austerity_error {
      const char *call;
      int errnum;
      const char *english;
    } first;

    unsigned int abort : 1;

    void (*callback)(void *, graph_builder_t *, const error_t *);
    void *user;
  } err;

  struct allocator {
    void *(*alloc)(void *, size_t);
    void (*free)(void *, void *);
    void *user;

    struct argv_arena *argv_arena;
    struct env_arena *env_arena;
    struct func_arena *func_arena;
  } a;
};

#endif
