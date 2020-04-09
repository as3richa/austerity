#include "common.h"
#include "dsl.h"

struct austerity_func {
  const char *name;
  int (*callback)(void *, const int *, size_t, const int *, size_t);
  environment_t *env;
  size_t n_out;
};
