#ifndef FUNC_H
#define FUCN_H

#include "common.h"

struct austerity_func {
  char *name;
  int (*callback)(void *, const int *, size_t, const int *, size_t);
  size_t n_out;
};

void destroy_func(graph_builder_t *g, func_t *func);

#endif
