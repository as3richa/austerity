#ifndef GRAPH_BUILDER_H
#define GRAPH_BUILDER_H

#include "allocator.h"
#include "common.h"
#include "errors.h"
#include "graph.h"

struct austerity_graph_builder {
  environment_t *default_env;
  struct graph gr;
  struct errors err;
  allocator_t a;
};

#endif
