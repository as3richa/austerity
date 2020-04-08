#include "common.h"
#include "dsl.h"
#include "graph-builder.h"

static stream_processor_t *emplace_source(graph_builder_t *g, const char *api_fn_name);

stream_t *fd_source(graph_builder_t *g, int fd) {
  INVAL_CHECK(g, fd < 0, "fd is negative", NULL);

  stream_processor_t *sp = emplace_source(g, __func__);

  if (sp == NULL) {
    return NULL;
  }

  sp->type = SP_FD_SOURCE;
  sp->u.source.fd = fd;

  return &sp->out[0];
}

stream_t *path_source(graph_builder_t *g, const char *path) {
  NULL_CHECK(g, path, NULL);

  char *my_path = copy_str(g, path, __func__);

  if (my_path == NULL) {
    return NULL;
  }

  stream_processor_t *sp = emplace_source(g, __func__);

  if (sp == NULL) {
    ifree(g, my_path);
    return NULL;
  }

  sp->type = SP_PATH_SOURCE;
  sp->u.source.path = my_path;

  return &sp->out[0];
}

stream_t *c_file_source(graph_builder_t *g, FILE *c_file) {
  NULL_CHECK(g, c_file, NULL);

  stream_processor_t *sp = emplace_source(g, __func__);

  if (sp == NULL) {
    return NULL;
  }

  sp->type = SP_C_FILE_SOURCE;
  sp->u.source.c_file = c_file;

  return &sp->out[0];
}

stream_t *str_source(graph_builder_t *g, const char *str) {
  return buffer_source(g, str, strlen(str));
}

stream_t *static_str_source(graph_builder_t *g, const char *str) {
  return static_buffer_source(g, str, strlen(str));
}

stream_t *buffer_source(graph_builder_t *g, const char *buffer, size_t size) {
  NULL_CHECK(g, buffer, NULL);

  char *my_buffer = copy_buffer(g, buffer, size, __func__);

  if (my_buffer == NULL) {
    return NULL;
  }

  stream_processor_t *sp = emplace_source(g, __func__);

  if (sp == NULL) {
    ifree(g, my_buffer);
    return NULL;
  }

  sp->type = SP_BUFFER_SOURCE;
  sp->u.source.buf = (struct buffer){my_buffer, size};

  return &sp->out[0];
}

stream_t *static_buffer_source(graph_builder_t *g, const char *buffer, size_t size) {
  NULL_CHECK(g, buffer, NULL);

  stream_processor_t *sp = emplace_source(g, __func__);

  if (sp == NULL) {
    return NULL;
  }

  sp->type = SP_STATIC_BUFFER_SOURCE;
  sp->u.source.sbuf = (struct static_buffer){buffer, size};

  return &sp->out[0];
}

static stream_processor_t *emplace_source(graph_builder_t *g, const char *api_fn_name) {
  return emplace_stream_processor(NULL, g, NULL, 0, 1, api_fn_name);
}
