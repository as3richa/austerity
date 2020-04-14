#ifndef ARGV_H
#define ARGV_H

#include "common.h"

struct argv_arg;

#define TYPE_ONLY
#define NAME argv_vec
#define CONTAINED_TYPE struct argv_arg
#include "vec.h"

struct austerity_argv {
  graph_builder_t *g;
  argv_vec_t args;

  unsigned int used : 1;
  unsigned int has_inputs : 1;
  unsigned int has_outputs : 1;
};

void initialize_argv(argv_t *argv, graph_builder_t *g);
void destroy_argv(argv_t *argv, allocator_t *alc);

#endif
