#include "graph-builder.h"

static void *default_alloc(size_t size, void *user);
static void default_free(void *ptr, void *user);

static void default_abort_on_error_callback(error_t *err, void *user);

static void
record_error(graph_builder_t *g, const char *api_fn_name, int errnum, const char *english);

graph_builder_t *create_graph_builder(void) {
  return create_graph_builder_a(default_alloc, default_free, NULL);
}

graph_builder_t *
create_graph_builder_a(void *(*alloc)(size_t, void *), void (*free)(void *, void *), void *user) {
  graph_builder_t *g = (*alloc)(sizeof(graph_builder_t), user);

  if (g == NULL) {
    return NULL;
  }

  g->default_env = NULL;
  g->sps = (struct stream_processor_vec){NULL, 0, 0};
  g->n_taps = 0;
  g->error.errnum = 0;
  g->abort_on_error = (struct abort_on_error){0, NULL, NULL};
  g->env_arena = NULL;
  g->a = (struct allocator){alloc, free, NULL};

  return g;
}

void graph_builder_abort_on_error(graph_builder_t *g) {
  graph_builder_abort_on_error_c(g, default_abort_on_error_callback, stderr);
}

void graph_builder_abort_on_error_c(graph_builder_t *g,
                                    void (*callback)(error_t *, void *),
                                    void *user) {
  g->abort_on_error = (struct abort_on_error){1, callback, user};
}

void destroy_graph_builder(graph_builder_t *g) {
  ifree(g, g->sps.ary); // FIXME: destroy stream processors
  DESTROY_ARENA(environment_t, g, &g->env_arena, destroy_environment);
  ifree(g, g);
}

environment_t *create_environment(graph_builder_t *g) {
  environment_t *env;
  ARENA_ALLOC(environment_t, &env, g, &g->env_arena, NULL);

  printf("%p\n", env);

  initialize_environment(env, g);
  return env;
}

void record_einval(graph_builder_t *g, const char *api_fn_name, const char *english) {
  record_error(g, api_fn_name, EINVAL, english);
}

void *ialloc(graph_builder_t *g, size_t size, const char *api_fn_name) {
  void *ptr = (*g->a.alloc)(size, g->a.user);

  if (ptr == NULL) {
    record_error(g, api_fn_name, errno, "cannot allocate memory");
    return NULL;
  }

  return ptr;
}

void *irealloc(graph_builder_t *g,
               void *ptr,
               size_t elem_size,
               size_t size,
               size_t prev,
               const char *api_fn_name) {
  void *next = ialloc(g, elem_size * size, api_fn_name);

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

  (*g->a.free)(ptr, g->a.user);
}

static void *default_alloc(size_t size, void *user) {
  (void)user;
  return malloc(size);
}

static void default_free(void *ptr, void *user) {
  (void)user;
  free(ptr);
}

static void default_abort_on_error_callback(error_t *err, void *user) {
  fprintf(user,
          "austerity: %s: %s (errno %d); aborting\n",
          err->api_fn_name,
          err->english,
          err->errnum);

  fflush(user);
}

static void
record_error(graph_builder_t *g, const char *api_fn_name, int errnum, const char *english) {
  error_t *error = &g->error;

  if (error->errnum == 0) {
    if (errnum == 0) {
      errnum = -1;
    }

    *error = (error_t){api_fn_name, errnum, english};
  }

  struct abort_on_error *a = &g->abort_on_error;

  if (a->active) {
    if (a->callback != NULL) {
      (*a->callback)(error, a->user);
    }

    abort();
  }
}

char *copy_buffer(graph_builder_t *g, const char *buffer, size_t size, const char *api_fn_name) {
  char *copy = ialloc(g, size, api_fn_name);

  if (copy == NULL) {
    return NULL;
  }

  memcpy(copy, buffer, size);

  return copy;
}

char *copy_str(graph_builder_t *g, const char *str, const char *api_fn_name) {
  return copy_buffer(g, str, strlen(str) + 1, api_fn_name);
}
