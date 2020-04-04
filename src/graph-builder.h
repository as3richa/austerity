#ifndef GRAPH_BUILDER_H
#define GRAPH_BUILDER_H

#define AUSTERITY_ABBREV

#include "austerity.h"
#include "environment.h"

#include <stdint.h>

typedef uint_least32_t st_size_t;

////////////////////////
typedef struct transformer transformer_t;
typedef struct source_data source_data_t;

struct austerity_graph_builder {
  error_t error;

  struct abort_on_error {
    unsigned int active : 1;
    void (*callback)(error_t *, void *);
    void *user;
  } abort_on_error;

  struct transformer_vec {
    transformer_t *ary;
    st_size_t size;
    st_size_t capacity;
  } transformers;

  struct source_data_vec {
    source_data_t *ary;
    st_size_t size;
    st_size_t capacity;
  } source_data;

  struct env_list {
    struct env_list *next;
    environment_t env;
  } * envs;

  environment_t *default_env;

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
