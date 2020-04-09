#include "graph-builder.h"
#include "errors.h"

static void *default_alloc(void *user, size_t size);
static void default_free(void *user, void *ptr);

graph_builder_t *create_graph_builder(void) {
  return create_graph_builder_a(default_alloc, default_free, NULL);
}

graph_builder_t *
create_graph_builder_a(void *(*alloc)(void *, size_t), void (*free)(void *, void *), void *user) {
  graph_builder_t *g = (*alloc)(user, sizeof(graph_builder_t));

  if (g == NULL) {
    return NULL;
  }

  g->default_env = NULL;
  initialize_dsl_state(&g->dsl);
  initialize_errors(&g->err);
  initialize_allocator(&g->a, alloc, free, user);

  return g;
}

void destroy_graph_builder(graph_builder_t *g) {
  destroy_dsl_state(g, &g->dsl);
  destroy_allocator(g, &g->a);
  ifree(g, g);
}

int set_default_environment(graph_builder_t *g, environment_t *env) {
  if (env == NULL) {
    record_einval(g, __func__, "env is NULL");
    return -1;
  }

  g->default_env = env;
  return 0;
}

static void *default_alloc(void *user, size_t size) {
  (void)user;
  return malloc(size);
}

static void default_free(void *user, void *ptr) {
  (void)user;
  free(ptr);
}
