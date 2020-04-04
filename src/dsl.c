#define AUSTERITY_ABBREV

#include "austerity.h"
#include "graph-builder.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

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
  TT_FUNCTION
} transformer_type_t;

struct transformer {
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
  } data;
};

struct source_data {
  unsigned int tap : 1;

  union {
    struct {
      st_size_t tf;
      st_size_t index;
    } tf_out;

    struct {
      unsigned int needs_path : 1;
      source_t out;
    } tap;
  } data;
};

struct austerity_argv_t {
  int fixme;
};

static source_t
create_sources(graph_builder_t *g, size_t tf_index, size_t n, const char *api_fn_name);

static transformer_t *
push_transformer(source_t *first, graph_builder_t *g, size_t n_out, const char *api_fn_name) {
  struct transformer_vec *tfs = &g->transformers;

  assert(tfs->size <= tfs->capacity);

  if (tfs->size == tfs->capacity) {
    const size_t capacity = 2 * tfs->capacity + 1;
    const size_t elem_size = sizeof(transformer_t);

    transformer_t *ary = irealloc(g, tfs->ary, elem_size, capacity, tfs->capacity, api_fn_name);

    if (ary == NULL) {
      return NULL;
    }

    tfs->ary = ary;
    tfs->capacity = capacity;
  }

  assert(tfs->size < tfs->capacity);

  const source_t sources = create_sources(g, tfs->size, n_out, api_fn_name);

  if (sources == NIL_SOURCE) {
    return NULL;
  }

  *first = sources;
  return &tfs->ary[tfs->size++];
}

static source_t
create_sources(graph_builder_t *g, size_t tf_index, size_t n, const char *api_fn_name) {
  struct source_data_vec *source_data = &g->source_data;

  assert(source_data->size <= source_data->capacity);

  if (source_data->size + n > source_data->capacity) {
    const size_t capacity = 2 * source_data->capacity + n;
    source_data_t *ary = irealloc(
        g, source_data->ary, sizeof(source_data_t), capacity, source_data->capacity, api_fn_name);

    if (ary == NULL) {
      return NIL_SOURCE;
    }

    source_data->ary = ary;
    source_data->capacity = capacity;
  }

  assert(source_data->size + n <= source_data->capacity);

  for (size_t i = 0; i < n; i++) {
    source_data_t *fixme = &source_data->ary[source_data->size + i];
    (void)fixme;
    (void)tf_index;
  }

  const size_t result = source_data->size;
  source_data->size += n;

  return result;
}

source_t fd_source(graph_builder_t *g, int fd) {
  if (fd < 0) {
    record_einval(g, __func__, "fd is negative");
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
    record_einval(g, __func__, "path is NULL");
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
    record_einval(g, __func__, "file is NULL");
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
    record_einval(g, __func__, "str is NULL");
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
    record_einval(g, __func__, "data is NULL");
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
    record_einval(g, __func__, "str is NULL");
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
    record_einval(g, __func__, "data is NULL");
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
