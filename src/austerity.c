#define AUSTERITY_ABBREV

#include "austerity.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef uint_least32_t st_size_t;

typedef enum {
  TT_FD_SOURCE,
  TT_FILE_SOURCE,
  TT_STDIO_SOURCE,
  TT_STR_SOURCE,
  TT_BUFFER_SOURCE,
  TT_STATIC_STR_SOURCE,
  TT_STATIC_BUFFER_SOURCE,
  TT_FD_SINK,
  TT_FILE_SINK,
  TT_STDIO_SINK,
  TT_COMMAND,
  TT_FUNCTION,
  TT_TEE
} transformer_type_t;

typedef struct {
  transformer_type_t type;

  union {
    int fd;

    struct {
      const char *path;
      int flags;
    } file;

    FILE *stdio_file;

    struct {
      char *data;
      st_size_t size;
    } buffer;

    struct {
      const char *data;
      st_size_t size;
    } static_buffer;

    struct {
      char *path;
      argv_t *argv;
      environment_t *env;
      source_t stdin;
    } command;

    struct {
      int fixme;
    } function;

    struct {
      st_size_t tf_index;
      st_size_t out_index;
    } tee;
  } data;

  struct transformer *next;
} transformer_t;

typedef struct {
  st_size_t tf_index;
  st_size_t out_index;
} source_data_t;

struct austerity_environment {
  graph_builder_t *g;

  char *wd;

  int clearenv;

  struct un_setenv_op_vec {
    struct un_setenv_op {
      unsigned int set : 1;
      unsigned int overwrite : 1;
      char *name;
      char *value;
    } * ary;

    size_t size;
    size_t capacity;
  } un_setenv_ops;

  uid_t ruid, euid;
  gid_t rgid, egid;
};

struct austerity_argv_t {
  int fixme;
};

struct austerity_graph_builder {
  error_t error;

  struct abort_on_error {
    unsigned int active : 1;
    void (*callback)(error_t *, void *);
    void *user;
  } abort_on_error;

  struct transformer_vec {
    transformer_t *ary;
    st_size_t size;
    st_size_t capacity;
  } transformers;

  struct source_data_vec {
    source_data_t *ary;
    st_size_t size;
    st_size_t capacity;
  } source_data;

  environment_t *default_env;

  struct allocator {
    void *(*alloc)(size_t, void *);
    void (*free)(void *, void *);
    void *user;
  } a;

  struct autofree_list {
    struct autofree_list *next;
    void (*destructor)(void *);

    union {
      size_t size;
      void *ptr;
    } data;
  } * autofree;
};

static void
record_error(graph_builder_t *g, const char *api_fn_name, int errnum, const char *english);

static void internal_free(void *ptr, graph_builder_t *g);

static void *internal_alloc(size_t size, graph_builder_t *g, const char *api_fn_name) {
  void *ptr = (*g->a.alloc)(size, g->a.user);

  if (ptr == NULL) {
    record_error(g, api_fn_name, errno, "cannot allocate memory");
    return NULL;
  }

  return ptr;
}

static void *internal_alloc_autofree(size_t size,
                                     void (*destructor)(void *),
                                     graph_builder_t *g,
                                     const char *api_fn_name) {
  struct autofree_list *af =
      internal_alloc(size + offsetof(struct autofree_list, data), g, api_fn_name);

  if (af == NULL) {
    return NULL;
  }

  af->next = g->autofree;
  af->destructor = destructor;

  g->autofree = af;

  return &af->data;
}

static void *internal_realloc(void *ptr,
                              size_t elem_size,
                              size_t capacity,
                              size_t prev,
                              graph_builder_t *g,
                              const char *api_fn_name) {
  void *next = internal_alloc(elem_size * capacity, g, api_fn_name);

  if (next == NULL) {
    return NULL;
  }

  memcpy(next, ptr, elem_size * prev);
  internal_free(ptr, g);

  return next;
}

static void internal_free(void *ptr, graph_builder_t *g) {
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

static void record_inval_error(graph_builder_t *g, const char *api_fn_name, const char *english) {
  record_error(g, api_fn_name, EINVAL, english);
}

static source_t
create_sources(graph_builder_t *g, size_t tf_index, size_t n, const char *api_fn_name);

static transformer_t *
push_transformer(source_t *first, graph_builder_t *g, size_t n_out, const char *api_fn_name) {
  struct transformer_vec *tx = &g->transformers;

  assert(tx->size <= tx->capacity);

  if (tx->size == tx->capacity) {
    const size_t capacity = 2 * tx->capacity + 1;
    transformer_t *ary =
        internal_realloc(tx->ary, sizeof(transformer_t), capacity, tx->capacity, g, api_fn_name);

    if (ary == NULL) {
      return NULL;
    }

    tx->ary = ary;
    tx->capacity = capacity;
  }

  assert(tx->size < tx->capacity);

  const source_t sources = create_sources(g, tx->size, n_out, api_fn_name);

  if (sources == NIL_SOURCE) {
    return NULL;
  }

  *first = sources;
  return &tx->ary[tx->size++];
}

static source_t
create_sources(graph_builder_t *g, size_t tf_index, size_t n, const char *api_fn_name) {
  struct source_data_vec *sx = &g->source_data;

  assert(sx->size <= sx->capacity);

  if (sx->size + n > sx->capacity) {
    const size_t capacity = 2 * sx->capacity + n;
    source_data_t *ary =
        internal_realloc(sx->ary, sizeof(source_data_t), capacity, sx->capacity, g, api_fn_name);

    if (ary == NULL) {
      return NIL_SOURCE;
    }

    sx->ary = ary;
    sx->capacity = capacity;
  }

  assert(sx->size + n <= sx->capacity);

  for (size_t i = 0; i < n; i++) {
    source_data_t *data = &sx->ary[sx->size + i];
    data->tf_index = tf_index;
    data->out_index = i;
  }

  const size_t result = sx->size;
  sx->size += n;

  return result;
}

static char *
copy_buffer(graph_builder_t *g, const char *buffer, size_t size, const char *api_fn_name) {
  char *copy = internal_alloc(size, g, api_fn_name);

  if (copy == NULL) {
    return NULL;
  }

  memcpy(copy, buffer, size);

  return copy;
}

static char *copy_str(graph_builder_t *g, const char *str, const char *api_fn_name) {
  return copy_buffer(g, str, strlen(str) + 1, api_fn_name);
}

graph_builder_t *create_graph_builder(void) {
  return create_graph_builder_a(default_alloc, default_free, NULL);
}

graph_builder_t *
create_graph_builder_a(void *(*alloc)(size_t, void *), void (*free)(void *, void *), void *user) {
  graph_builder_t *g = (*alloc)(sizeof(graph_builder_t), user);

  if (g == NULL) {
    return NULL;
  }

  g->abort_on_error = (struct abort_on_error){0, NULL, NULL};
  g->transformers = (struct transformer_vec){NULL, 0, 0};
  g->source_data = (struct source_data_vec){NULL, 0, 0};
  g->autofree = NULL;
  g->a = (struct allocator){alloc, free, NULL};

  return g;
}

static void default_abort_on_error_callback(error_t *err, void *user) {
  fprintf(user,
          "austerity: %s: %s (errno %d); aborting\n",
          err->api_fn_name,
          err->english,
          err->errnum);

  fflush(user);
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
  internal_free(g->source_data.ary, g);
  internal_free(g->transformers.ary, g);

  for (struct autofree_list *it = g->autofree, *next; it != NULL; it = next) {
    next = it->next;
    (*it->destructor)(&it->data);
    internal_free(it, g);
  }

  internal_free(g, g);
}

source_t fd_source(graph_builder_t *g, int fd) {
  if (fd < 0) {
    record_inval_error(g, __func__, "fd is negative");
    return NIL_SOURCE;
  }

  source_t out;
  transformer_t *t = push_transformer(&out, g, 1, __func__);

  if (t == NULL) {
    return NIL_SOURCE;
  }

  t->type = TT_FD_SOURCE;
  t->data.fd = fd;

  return out;
}

source_t file_source(graph_builder_t *g, const char *path, int flags) {
  if (path == NULL) {
    record_inval_error(g, __func__, "path is NULL");
    return NIL_SOURCE;
  }

  const char *my_path = copy_str(g, path, __func__);

  if (my_path == NULL) {
    return NIL_SOURCE;
  }

  source_t out;
  transformer_t *t = push_transformer(&out, g, 1, __func__);

  t->type = TT_FILE_SOURCE;
  t->data.file.path = my_path;
  t->data.file.flags = flags;

  return out;
}

source_t stdio_source(graph_builder_t *g, FILE *file) {
  if (file == NULL) {
    record_inval_error(g, __func__, "file is NULL");
    return NIL_SOURCE;
  }

  source_t out;
  transformer_t *t = push_transformer(&out, g, 1, __func__);

  t->type = TT_STDIO_SOURCE;
  t->data.stdio_file = file;

  return out;
}

source_t str_source(graph_builder_t *g, const char *str) {
  if (str == NULL) {
    record_inval_error(g, __func__, "str is NULL");
    return NIL_SOURCE;
  }

  const size_t size = strlen(str);
  char *my_str = copy_buffer(g, str, size, __func__);

  if (my_str == NULL) {
    return NIL_SOURCE;
  }

  source_t out;
  transformer_t *t = push_transformer(&out, g, 1, __func__);

  if (t == NULL) {
    return NIL_SOURCE;
  }

  t->type = TT_STR_SOURCE;
  t->data.buffer.data = my_str;
  t->data.buffer.size = size;

  return out;
}

source_t buffer_source(graph_builder_t *g, const char *data, size_t size) {
  if (data == NULL) {
    record_inval_error(g, __func__, "data is NULL");
    return NIL_SOURCE;
  }

  char *my_data = copy_buffer(g, data, size, __func__);

  if (my_data == NULL) {
    return NIL_SOURCE;
  }

  source_t out;
  transformer_t *t = push_transformer(&out, g, 1, __func__);

  if (t == NULL) {
    return NIL_SOURCE;
  }

  t->type = TT_BUFFER_SOURCE;
  t->data.buffer.data = my_data;
  t->data.buffer.size = size;

  return out;
}

source_t static_str_source(graph_builder_t *g, const char *str) {
  if (str == NULL) {
    record_inval_error(g, __func__, "str is NULL");
    return NIL_SOURCE;
  }

  source_t out;
  transformer_t *t = push_transformer(&out, g, 1, __func__);

  if (t == NULL) {
    return NIL_SOURCE;
  }

  t->type = TT_STATIC_STR_SOURCE;
  t->data.static_buffer.data = str;
  t->data.static_buffer.size = strlen(str);

  return out;
}

source_t static_buffer_source(graph_builder_t *g, const char *data, size_t size) {
  if (data == NULL) {
    record_inval_error(g, __func__, "data is NULL");
    return NIL_SOURCE;
  }

  source_t out;
  transformer_t *t = push_transformer(&out, g, 1, __func__);

  if (t == NULL) {
    return NIL_SOURCE;
  }

  t->type = TT_STATIC_BUFFER_SOURCE;
  t->data.static_buffer.data = data;
  t->data.static_buffer.size = size;

  return out;
}

static void destroy_environment(void *void_env);

environment_t *create_environment(graph_builder_t *g) {
  environment_t *env =
      internal_alloc_autofree(sizeof(environment_t), destroy_environment, g, __func__);

  if (env == NULL) {
    return NULL;
  }

  *env = (environment_t){g, NULL, 1, (struct un_setenv_op_vec){NULL, 0, 0}, -1, -1, -1, -1};

  return env;
}

static void destroy_environment(void *void_env) {
  environment_t *env = void_env;
  graph_builder_t *g = env->g;

  internal_free(env->wd, g);

  const struct un_setenv_op_vec *ops = &env->un_setenv_ops;

  for (size_t i = 0; i < ops->capacity; i++) {
    internal_free(ops->ary[i].name, g);
    internal_free(ops->ary[i].value, g);
  }

  internal_free(ops->ary, g);

  internal_free(env, g);
}

int set_default_environment(graph_builder_t *g, environment_t *env) {
  if (env == NULL) {
    record_inval_error(g, __func__, "env is NULL");
    return -1;
  }

  g->default_env = env;
  return 0;
}

int environment_setwd(environment_t *env, const char *path) {
  if (env == NULL) {
    return -1;
  }

  if (path == NULL) {
    record_inval_error(env->g, __func__, "path is NULL");
    return -1;
  }

  char *my_path = copy_str(env->g, path, __func__);

  if (my_path == NULL) {
    return -1;
  }

  env->wd = my_path;
  return 0;
}

int environment_preserve_env(environment_t *env) {
  if (env == NULL) {
    return -1;
  }

  env->clearenv = 0;
  return 0;
}

int environment_clearenv(environment_t *env) {
  if (env == NULL) {
    return -1;
  }

  env->clearenv = 1;
  env->un_setenv_ops.size = 0;
  return 0;
}

struct un_setenv_op *emplace_un_setenv_op(environment_t *env, const char *api_fn_name) {
  struct un_setenv_op_vec *ops = &env->un_setenv_ops;

  assert(ops->size <= ops->capacity);

  if (ops->size == ops->capacity) {
    const size_t capacity = 2 * ops->capacity + 1;
    struct un_setenv_op *ary = internal_realloc(
        ops->ary, sizeof(struct un_setenv_op), capacity, ops->capacity, env->g, api_fn_name);

    if (ary == NULL) {
      return NULL;
    }

    ops->ary = ary;
    ops->capacity = capacity;
  }

  assert(ops->size < ops->capacity);

  return &ops->ary[ops->size++];
}

int environment_setenv(environment_t *env, const char *name, const char *value, int overwrite) {
  if (env == NULL) {
    return -1;
  }

  graph_builder_t *g = env->g;

  if (name == NULL) {
    record_inval_error(g, __func__, "name is NULL");
    return -1;
  }

  if (value == NULL) {
    record_inval_error(g, __func__, "value is NULL");
    return -1;
  }

  char *my_name = copy_str(g, name, __func__);

  if (my_name == NULL) {
    return -1;
  }

  char *my_value = copy_str(g, value, __func__);

  if (my_value == NULL) {
    internal_free(my_name, g);
    return -1;
  }

  struct un_setenv_op *op = emplace_un_setenv_op(env, __func__);

  if (op == NULL) {
    internal_free(my_name, g);
    internal_free(my_value, g);
    return -1;
  }

  *op = (struct un_setenv_op){1, overwrite, my_name, my_value};
  return 0;
}

int environment_unsetenv(environment_t *env, const char *name) {
  if (env == NULL) {
    return -1;
  }

  graph_builder_t *g = env->g;

  if (name == NULL) {
    record_inval_error(g, __func__, "name is NULL");
    return -1;
  }

  char *my_name = copy_str(g, name, __func__);

  if (my_name == NULL) {
    return -1;
  }

  struct un_setenv_op *op = emplace_un_setenv_op(env, __func__);

  if (op == NULL) {
    internal_free(my_name, g);
    return -1;
  }

  *op = (struct un_setenv_op){0, 0, my_name, NULL};
  return 0;
}

int environment_setreuid(environment_t *env, uid_t ruid, uid_t euid) {
  if (env == NULL) {
    return -1;
  }

  env->ruid = ruid;
  env->euid = euid;
  return 0;
}

int environment_setregid(environment_t *env, gid_t rgid, gid_t egid) {
  if (env == NULL) {
    return -1;
  }

  env->rgid = rgid;
  env->egid = egid;
  return 0;
}
