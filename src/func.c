#include "func.h"
#include "alloc.h"

func_t *create_func(graph_builder_t *g,
                    const char *name,
                    int (*callback)(void *, const int *, size_t, const int *, size_t),
                    size_t n_out) {
  char *my_name = g_copy_str(g, name, __func__);

  if (my_name == NULL) {
    return NULL;
  }

  func_t *func = g_alloc_func(g, __func__);

  if (func == NULL) {
    g_free(g, my_name);
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
  g_free(g, func->name);
}
