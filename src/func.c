#include "func.h"
#include "alloc.h"

func_t *create_func(graph_builder_t *g,
                    const char *name,
                    int (*callback)(void *, const int *, size_t, const int *, size_t),
                    size_t n_out) {
  char *my_name = copy_str(g, name, __func__);

  if (my_name == NULL) {
    return NULL;
  }

  func_t *func = alloc_func(g, __func__);

  if (func == NULL) {
    ifree(g, my_name);
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

void destroy_func(graph_builder_t *g, func_t *func) {
  ifree(g, func->name);
}
