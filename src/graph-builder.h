#ifndef GRAPH_BUILDER_H
#define GRAPH_BUILDER_H

#include "alloc.h"
#include "common.h"
#include "errors.h"
#include "graph.h"

struct austerity_graph_builder {
  environment_t *default_env;
  struct graph gr;
  struct errors err;
  struct allocator a;
};

#endif
