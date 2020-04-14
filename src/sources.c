#include "common.h"
#include "errors.h"
#include "graph-builder.h"
#include "graph.h"

static stream_processor_t *emplace_source(graph_t *graph, allocator_t *alc, errors_t *errors);

static stream_t my_buffer_source(
    graph_t *graph, const char *buffer, size_t size, allocator_t *alc, errors_t *errors);

static stream_t my_static_buffer_source(
    graph_t *graph, const char *buffer, size_t size, allocator_t *alc, errors_t *errors);

stream_t fd_source(graph_builder_t *g, int fd) {
  errors_t *errors = &g->errors;

  CHECK_INVAL(fd < 0, "fd is negative", NIL_STREAM, errors);

  stream_processor_t *sp = emplace_source(&g->graph, &g->alc, errors);

  if (sp == NULL) {
    return NIL_STREAM;
  }

  sp->type = SP_FD_SOURCE;
  sp->u.source.u.fd = fd;

  return sp->u.source.out;
}

stream_t path_source(graph_builder_t *g, const char *path) {
  errors_t *errors = &g->errors;
  allocator_t *alc = &g->alc;

  CHECK_IF_NULL(path, NIL_STREAM, errors);

  char *my_path = a_copy_str(alc, path, errors);

  if (my_path == NULL) {
    return NIL_STREAM;
  }

  stream_processor_t *sp = emplace_source(&g->graph, alc, errors);

  if (sp == NULL) {
    a_free(alc, my_path);
    return NIL_STREAM;
  }

  sp->type = SP_PATH_SOURCE;
  sp->u.source.u.path = my_path;

  return sp->u.source.out;
}

stream_t c_file_source(graph_builder_t *g, FILE *c_file) {
  errors_t *errors = &g->errors;

  CHECK_IF_NULL(c_file, NIL_STREAM, errors);

  stream_processor_t *sp = emplace_source(&g->graph, &g->alc, errors);

  if (sp == NULL) {
    return NIL_STREAM;
  }

  sp->type = SP_C_FILE_SOURCE;
  sp->u.source.u.c_file = c_file;

  return sp->u.source.out;
}

stream_t str_source(graph_builder_t *g, const char *str) {
  return my_buffer_source(&g->graph, str, strlen(str), &g->alc, &g->errors);
}

stream_t static_str_source(graph_builder_t *g, const char *str) {
  return my_static_buffer_source(&g->graph, str, strlen(str), &g->alc, &g->errors);
}

stream_t buffer_source(graph_builder_t *g, const char *buffer, size_t size) {
  return my_buffer_source(&g->graph, buffer, size, &g->alc, &g->errors);
}

stream_t static_buffer_source(graph_builder_t *g, const char *buffer, size_t size) {
  return my_static_buffer_source(&g->graph, buffer, size, &g->alc, &g->errors);
}

static stream_processor_t *emplace_source(graph_t *graph, allocator_t *alc, errors_t *errors) {
  stream_t out;
  stream_processor_t *sp = create_stream_processor(graph, NULL, NULL, 0, &out, 1, alc, errors);

  if (sp == NULL) {
    return NULL;
  }

  sp->u.source.out = out;
  return sp;
}

static stream_t my_buffer_source(
    graph_t *graph, const char *buffer, size_t size, allocator_t *alc, errors_t *errors) {
  CHECK_IF_NULL(buffer, NIL_STREAM, errors);

  char *my_buffer = a_copy_buffer(alc, buffer, size, errors);

  if (my_buffer == NULL) {
    return NIL_STREAM;
  }

  stream_processor_t *sp = emplace_source(graph, alc, errors);

  if (sp == NULL) {
    a_free(alc, my_buffer);
    return NIL_STREAM;
  }

  sp->type = SP_BUFFER_SOURCE;
  sp->u.source.u.buf = (struct buffer){my_buffer, size};

  return sp->u.source.out;
}

static stream_t my_static_buffer_source(
    graph_t *graph, const char *buffer, size_t size, allocator_t *alc, errors_t *errors) {
  CHECK_IF_NULL(buffer, NIL_STREAM, errors);

  stream_processor_t *sp = emplace_source(graph, alc, errors);

  if (sp == NULL) {
    return NIL_STREAM;
  }

  sp->type = SP_STATIC_BUFFER_SOURCE;
  sp->u.source.u.sbuf = (struct static_buffer){buffer, size};

  return sp->u.source.out;
}
