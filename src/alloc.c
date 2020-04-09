#include "alloc.h"
#include "argv.h"
#include "environment.h"
#include "errors.h"

struct austerity_func {
  int x;
};

#include "graph-builder.h"

#define NAME env_arena
#define CONTAINED_TYPE environment_t
#define DESTRUCTOR destroy_environment
#include "arena.h"

#define NAME argv_arena
#define CONTAINED_TYPE argv_t
#define CONSTRUCTOR initialize_argv
#define DESTRUCTOR destroy_argv
#include "arena.h"

#define NAME func_arena
#define CONTAINED_TYPE func_t
#undef DESTRUCTOR // FIXME
#include "arena.h"

void initialize_allocator(struct allocator *a,
                          void *(*alloc)(void *, size_t),
                          void (*free)(void *, void *),
                          void *user) {
  a->alloc = alloc;
  a->free = free;
  a->user = user;
  a->argv_arena = NULL;
  a->env_arena = NULL;
  a->func_arena = NULL;
}

void destroy_allocator(graph_builder_t *g, struct allocator *a) {
  destroy_argv_arena(g, &a->argv_arena);
  destroy_env_arena(g, &a->env_arena);
  destroy_func_arena(g, &a->func_arena);
}

void *ialloc(graph_builder_t *g, size_t size, const char *call) {
  void *ptr = (*g->a.alloc)(g->a.user, size);

  if (ptr == NULL) {
    record_alloc_failure(g, call);
    return NULL;
  }

  return ptr;
}

void *irealloc(
    graph_builder_t *g, void *ptr, size_t elem_size, size_t size, size_t prev, const char *call) {
  void *next = ialloc(g, elem_size * size, call);

  if (next == NULL) {
    return NULL;
  }

  memcpy(next, ptr, elem_size * prev);
  ifree(g, ptr);

  return next;
}

void ifree(graph_builder_t *g, void *ptr) {
  if (ptr == NULL) {
    return;
  }

  (*g->a.free)(g->a.user, ptr);
}

char *copy_buffer(graph_builder_t *g, const char *buffer, size_t size, const char *call) {
  char *copy = ialloc(g, size, call);

  if (copy == NULL) {
    return NULL;
  }

  memcpy(copy, buffer, size);

  return copy;
}

char *copy_str(graph_builder_t *g, const char *str, const char *call) {
  return copy_buffer(g, str, strlen(str) + 1, call);
}

argv_t *alloc_argv(graph_builder_t *g, const char *call) {
  return argv_arena_alloc(g, &g->a.argv_arena, call);
}

environment_t *alloc_env(graph_builder_t *g, const char *call) {
  return env_arena_alloc(g, &g->a.env_arena, call);
}

func_t *alloc_func(graph_builder_t *g, const char *call) {
  return func_arena_alloc(g, &g->a.func_arena, call);
}
