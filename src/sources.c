#include "common.h"
#include "errors.h"
#include "graph-builder.h"
#include "graph.h"

static stream_processor_t *emplace_source(graph_builder_t *g, const char *call);

static stream_t
my_buffer_source(graph_builder_t *g, const char *buffer, size_t size, const char *call);
static stream_t
my_static_buffer_source(graph_builder_t *g, const char *buffer, size_t size, const char *call);

stream_t fd_source(graph_builder_t *g, int fd) {
  INVAL_CHECK(g, fd < 0, "fd is negative", NIL_STREAM, __func__);

  stream_processor_t *sp = emplace_source(g, __func__);

  if (sp == NULL) {
    return NIL_STREAM;
  }

  sp->type = SP_FD_SOURCE;
  sp->u.source.u.fd = fd;

  return sp->u.source.out;
}

stream_t path_source(graph_builder_t *g, const char *path) {
  NULL_CHECK(g, path, NIL_STREAM, __func__);

  char *my_path = copy_str(g, path, __func__);

  if (my_path == NULL) {
    return NIL_STREAM;
  }

  stream_processor_t *sp = emplace_source(g, __func__);

  if (sp == NULL) {
    ifree(g, my_path);
    return NIL_STREAM;
  }

  sp->type = SP_PATH_SOURCE;
  sp->u.source.u.path = my_path;

  return sp->u.source.out;
}

stream_t c_file_source(graph_builder_t *g, FILE *c_file) {
  NULL_CHECK(g, c_file, NIL_STREAM, __func__);

  stream_processor_t *sp = emplace_source(g, __func__);

  if (sp == NULL) {
    return NIL_STREAM;
  }

  sp->type = SP_C_FILE_SOURCE;
  sp->u.source.u.c_file = c_file;

  return sp->u.source.out;
}

stream_t str_source(graph_builder_t *g, const char *str) {
  return my_buffer_source(g, str, strlen(str), __func__);
}

stream_t static_str_source(graph_builder_t *g, const char *str) {
  return my_static_buffer_source(g, str, strlen(str), __func__);
}

stream_t buffer_source(graph_builder_t *g, const char *buffer, size_t size) {
  return my_buffer_source(g, buffer, size, __func__);
}

stream_t static_buffer_source(graph_builder_t *g, const char *buffer, size_t size) {
  return my_static_buffer_source(g, buffer, size, __func__);
}

static stream_processor_t *emplace_source(graph_builder_t *g, const char *call) {
  stream_t out;
  stream_processor_t *sp = create_stream_processor(g, NULL, NULL, 0, &out, 1, call);

  if (sp == NULL) {
    return NULL;
  }

  sp->u.source.out = out;
  return sp;
}

static stream_t
my_buffer_source(graph_builder_t *g, const char *buffer, size_t size, const char *call) {
  NULL_CHECK(g, buffer, NIL_STREAM, call);

  char *my_buffer = copy_buffer(g, buffer, size, call);

  if (my_buffer == NULL) {
    return NIL_STREAM;
  }

  stream_processor_t *sp = emplace_source(g, call);

  if (sp == NULL) {
    ifree(g, my_buffer);
    return NIL_STREAM;
  }

  sp->type = SP_BUFFER_SOURCE;
  sp->u.source.u.buf = (struct buffer){my_buffer, size};

  return sp->u.source.out;
}

static stream_t
my_static_buffer_source(graph_builder_t *g, const char *buffer, size_t size, const char *call) {
  NULL_CHECK(g, buffer, NIL_STREAM, call);

  stream_processor_t *sp = emplace_source(g, call);

  if (sp == NULL) {
    return NIL_STREAM;
  }

  sp->type = SP_STATIC_BUFFER_SOURCE;
  sp->u.source.u.sbuf = (struct static_buffer){buffer, size};

  return sp->u.source.out;
}
