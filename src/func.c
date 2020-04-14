#include "func.h"
#include "allocator.h"
#include "graph-builder.h"

func_t *create_func(graph_builder_t *g,
                    const char *name,
                    int (*callback)(void *, const int *, size_t, const int *, size_t),
                    size_t n_out) {
  allocator_t *alc = &g->alc;
  errors_t *errors = &g->errors;

  char *my_name = a_copy_str(alc, name, errors);

  if (my_name == NULL) {
    return NULL;
  }

  func_t *func = a_alloc_func(alc, errors);

  if (func == NULL) {
    a_free(alc, my_name);
    return NULL;
  }

  *func = (func_t){my_name, callback, n_out};
  return func;
}

size_t func_n_out(const func_t *func) {
  if (func == NULL) {
    return 0;
  }

  return func->n_out;
}

void destroy_func(func_t *func, allocator_t *alc) {
  a_free(alc, func->name);
}
