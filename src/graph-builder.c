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
  initialize_graph(&g->graph);
  initialize_errors(&g->errors);
  initialize_allocator(&g->alc, alloc, free, user);

  return g;
}

void destroy_graph_builder(graph_builder_t *g) {
  allocator_t *alc = &g->alc;
  destroy_graph(&g->graph, alc);
  destroy_allocator_arenas(alc);
  a_free(alc, g);
}

int set_default_environment(graph_builder_t *g, environment_t *env) {
  CHECK_IF_NULL(env, -1, &g->errors);
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
