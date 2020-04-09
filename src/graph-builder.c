#include "argv.h"
#include "environment.h"
#include "graph-builder.h"
#include "common.h"

#define NAME env_arena
#define CONTAINED_TYPE environment_t
#define CONSTRUCTOR initialize_environment
#define DESTRUCTOR destroy_environment
#include "arena.h"

#define NAME argv_arena
#define CONTAINED_TYPE argv_t
#define CONSTRUCTOR initialize_argv
#define DESTRUCTOR destroy_argv
#include "arena.h"

struct austerity_graph_builder {
  environment_t *default_env;

  dsl_state_t dsl;

  error_t error;

  struct abort_on_error {
    unsigned int active : 1;
    void (*callback)(error_t *, void *);
    void *user;
  } abort_on_error;

  struct allocator {
    void *(*alloc)(size_t, void *);
    void (*free)(void *, void *);
    void *user;
  } a;

  env_arena_t *env_arena;
  argv_arena_t *argv_arena;
};

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

  initialize_dsl_state(&g->dsl);

  g->error.errnum = 0;
  g->abort_on_error = (struct abort_on_error){0, NULL, NULL};

  g->a = (struct allocator){alloc, free, NULL};
  initialize_env_arena(&g->env_arena);
  initialize_argv_arena(&g->argv_arena);

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
  destroy_dsl_state(g, &g->dsl);
  destroy_env_arena(g, &g->env_arena);
  destroy_argv_arena(g, &g->argv_arena);
  ifree(g, g);
}

environment_t *create_environment(graph_builder_t *g) {
  return env_arena_alloc(g, &g->env_arena, __func__);
}

argv_t *create_argv(graph_builder_t *g) {
  return argv_arena_alloc(g, &g->argv_arena, __func__);
}

argv_t *create_argv_v(graph_builder_t *g, ...) {
  argv_t *argv = create_argv(g);

  if (argv == NULL) {
    return NULL;
  }

  va_list v;
  va_start(v, g);
  const int result = argv_push_strs_va(argv, v, __func__);
  va_end(v);

  if (result < 0) {
    destroy_argv(g, argv);
    // FIXME: stick argv back on the arena??
    return NULL;
  }

  return argv;
}

int set_default_environment(graph_builder_t *g, environment_t *env) {
  if (env == NULL) {
    record_einval(g, __func__, "env is NULL");
    return -1;
  }

  g->default_env = env;
  return 0;
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

dsl_state_t *graph_builder_dsl(graph_builder_t *g) {
  return &g->dsl;
}

environment_t *graph_builder_default_env(graph_builder_t *g) {
  return g->default_env;
}
