#ifndef FUNC_H
#define FUNC_H

#include "allocator.h"
#include "common.h"

struct austerity_func {
  char *name;
  int (*callback)(void *, const int *, size_t, const int *, size_t);
  size_t n_out;
};

void destroy_func(func_t *func, allocator_t *alc);

#endif
