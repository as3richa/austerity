#include "alloc.h"
#include "allocator.h"
#include "graph-builder.h"

#define CHECK_AND_RETURN_PTR(g, ptr)                                                               \
  do {                                                                                             \
    if ((ptr) == NULL) {                                                                           \
      record_alloc_failure((g), call);                                                             \
      return NULL;                                                                                 \
    }                                                                                              \
    return (ptr);                                                                                  \
  } while (0)

void *g_alloc(graph_builder_t *g, size_t size, const char *call) {
  void *ptr = a_alloc(&g->a, size);
  CHECK_AND_RETURN_PTR(g, ptr);
}

void *g_realloc(
    graph_builder_t *g, void *ptr, size_t elem_size, size_t size, size_t prev, const char *call) {
  void *next = a_realloc(&g->a, ptr, elem_size, size, prev);
  CHECK_AND_RETURN_PTR(g, next);
}

void g_free(graph_builder_t *g, void *ptr) {
  a_free(&g->a, ptr);
}

char *g_copy_buffer(graph_builder_t *g, const char *buffer, size_t size, const char *call) {
  char *copy = a_copy_buffer(&g->a, buffer, size);
  CHECK_AND_RETURN_PTR(g, copy);
}

char *g_copy_str(graph_builder_t *g, const char *str, const char *call) {
  char *copy = a_copy_str(&g->a, str);
  CHECK_AND_RETURN_PTR(g, copy);
}

argv_t *g_alloc_argv(graph_builder_t *g, const char *call) {
  argv_t *argv = a_alloc_argv(&g->a);
  CHECK_AND_RETURN_PTR(g, argv);
}

environment_t *g_alloc_env(graph_builder_t *g, const char *call) {
  environment_t *env = a_alloc_env(&g->a);
  CHECK_AND_RETURN_PTR(g, env);
}

func_t *g_alloc_func(graph_builder_t *g, const char *call) {
  func_t *func = a_alloc_func(&g->a);
  CHECK_AND_RETURN_PTR(g, func);
}
