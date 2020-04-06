#ifndef COMMANDS_H
#define COMMANDS_H

#include "common.h"

typedef union {
  char *str;
  stream_t *stream;
} argv_arg_t;

#define NAME argv_vec
#define CONTAINED_TYPE argv_arg_t
#include "vec.h"

struct austerity_argv {
  graph_builder_t *g;
  argv_vec_t args;
};

void initialize_argv(argv_t *argv, graph_builder_t *g);
void destroy_argv(graph_builder_t *g, argv_t *argv);

int argv_push_strs_va(austerity_argv_t *argv, va_list v, const char *api_fn_name);

#endif
