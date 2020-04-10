#ifndef ARGV_H
#define ARGV_H

#include "common.h"

typedef union argv_arg argv_arg_t;

#define TYPE_ONLY
#define NAME argv_vec
#define CONTAINED_TYPE argv_arg_t
#include "vec.h"

struct austerity_argv {
  graph_builder_t *g;
  argv_vec_t args;
};

void initialize_argv(graph_builder_t *g, argv_t *argv);
void destroy_argv(graph_builder_t *g, argv_t *argv);

#endif
