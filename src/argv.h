#ifndef ARGV_H
#define ARGV_H

#include "common.h"

union argv_arg;

#define TYPE_ONLY
#define NAME argv_vec
#define CONTAINED_TYPE union argv_arg
#include "vec.h"

struct austerity_argv {
  graph_builder_t *g;
  argv_vec_t args;
};

void initialize_argv(graph_builder_t *g, argv_t *argv);
void destroy_argv(graph_builder_t *g, argv_t *argv);

#endif
