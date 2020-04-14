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
  initialize_graph(&g->gr);
  initialize_errors(&g->errors);
  initialize_allocator(&g->a, alloc, free, user);

  return g;
}

void destroy_graph_builder(graph_builder_t *g) {
  destroy_graph(g, &g->gr);
  destroy_allocator_arenas(&g->a);
  a_free(&g->a, g);
}

int set_default_environment(graph_builder_t *g, environment_t *env) {
  NULL_CHECK(g, env, -1, __func__);
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
