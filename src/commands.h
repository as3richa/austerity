#ifndef COMMANDS_H
#define COMMANDS_H

#include "common.h"

union arg {
  char *str;
  stream_t *stream;
};

struct austerity_argv {
  graph_builder_t *g;

  struct {
    union arg *ary;
    st_size_t size;
    st_size_t capacity;
  } args;

  struct {
    size_t *ary;
    st_size_t size;
    st_size_t capacity;
  } is_stream;
};

void initialize_argv(argv_t *argv, graph_builder_t *g);
void destroy_argv(argv_t *argv);

int argv_push_strs_va(austerity_argv_t *argv, va_list v, const char *api_fn_name);

#endif
