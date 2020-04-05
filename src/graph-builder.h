#ifndef GRAPH_BUILDER_H
#define GRAPH_BUILDER_H

#include "common.h"
#include "environment.h"

typedef uint_least32_t st_size_t;
typedef st_size_t tap_t;

struct austerity_graph_builder {
  environment_t *default_env;

  struct stream_processor_vec {
    struct stream_processor *ary;
    size_t size;
    size_t capacity;
  } sps;

  tap_t n_taps;

  error_t error;

  struct abort_on_error {
    unsigned int active : 1;
    void (*callback)(error_t *, void *);
    void *user;
  } abort_on_error;

  struct env_list {
    struct env_list *next;
    environment_t env;
  } * envs;

  struct allocator {
    void *(*alloc)(size_t, void *);
    void (*free)(void *, void *);
    void *user;
  } a;
};

void record_einval(graph_builder_t *g, const char *api_fn_name, const char *english);

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
