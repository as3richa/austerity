#ifndef GRAPH_BUILDER_H
#define GRAPH_BUILDER_H

#include "commands.h"
#include "common.h"
#include "dsl.h"
#include "environment.h"

#define NAME stream_processor_vec
#define CONTAINED_TYPE stream_processor_t
#include "vec.h"

#define NAME env_arena
#define CONTAINED_TYPE environment_t
#include "arena.h"

#define NAME argv_arena
#define CONTAINED_TYPE argv_t
#include "arena.h"

struct austerity_graph_builder {
  environment_t *default_env;

  stream_processor_vec_t sps;
  tap_t n_taps;

  error_t error;

  struct abort_on_error {
    unsigned int active : 1;
    void (*callback)(error_t *, void *);
    void *user;
  } abort_on_error;

  struct allocator {
    void *(*alloc)(size_t, void *);
    void (*free)(void *, void *);
    void *user;
  } a;

  env_arena_t *env_arena;
  argv_arena_t *argv_arena;
};

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

#endif
