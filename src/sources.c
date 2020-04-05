#include "common.h"
#include "stream-processor.h"

static struct stream_processor *emplace_source(graph_builder_t *g, const char *api_fn_name);

#define NULL_CHECK(ident)                                                                          \
  do {                                                                                             \
    if ((ident) == NULL) {                                                                         \
      record_einval(g, __func__, (#ident " is NULL"));                                             \
      return NULL;                                                                                 \
    }                                                                                              \
  } while (0)

source_t *fd_source(graph_builder_t *g, int fd) {
  if (fd < 0) {
    record_einval(g, __func__, "fd is negative");
    return NULL;
  }

  struct stream_processor *sp = emplace_source(g, __func__);

  if (sp == NULL) {
    return NULL;
  }

  sp->type = SP_FD_SOURCE;
  sp->data.fd = fd;

  return &sp->out[0];
}

source_t *path_source(graph_builder_t *g, const char *path) {
  NULL_CHECK(path);

  char *my_path = copy_str(g, path, __func__);

  if (my_path == NULL) {
    return NULL;
  }

  struct stream_processor *sp = emplace_source(g, __func__);

  if (sp == NULL) {
    ifree(g, my_path);
    return NULL;
  }

  sp->type = SP_PATH_SOURCE;
  sp->data.path.path = my_path;

  return &sp->out[0];
}

source_t *c_file_source(graph_builder_t *g, FILE *c_file) {
  NULL_CHECK(c_file);

  struct stream_processor *sp = emplace_source(g, __func__);

  if (sp == NULL) {
    return NULL;
  }

  sp->type = SP_C_FILE_SOURCE;
  sp->data.c_file = c_file;

  return &sp->out[0];
}

source_t *str_source(graph_builder_t *g, const char *str) {
  return buffer_source(g, str, strlen(str));
}

source_t *static_str_source(graph_builder_t *g, const char *str) {
  return static_buffer_source(g, str, strlen(str));
}

source_t *buffer_source(graph_builder_t *g, const char *buffer, size_t size) {
  NULL_CHECK(buffer);

  char *my_buffer = copy_buffer(g, buffer, size, __func__);

  if (my_buffer == NULL) {
    return NULL;
  }

  struct stream_processor *sp = emplace_source(g, __func__);

  if (sp == NULL) {
    ifree(g, my_buffer);
    return NULL;
  }

  sp->type = SP_BUFFER_SOURCE;
  sp->data.buffer = (struct buffer){my_buffer, size};

  return &sp->out[0];
}

source_t *static_buffer_source(graph_builder_t *g, const char *buffer, size_t size) {
  NULL_CHECK(buffer);

  struct stream_processor *sp = emplace_source(g, __func__);

  if (sp == NULL) {
    return NULL;
  }

  sp->type = SP_STATIC_BUFFER_SOURCE;
  sp->data.static_buffer = (struct static_buffer){buffer, size};

  return &sp->out[0];
}

static struct stream_processor *emplace_source(graph_builder_t *g, const char *api_fn_name) {
  return emplace_stream_processor(NULL, g, NULL, 0, 1, api_fn_name);
}
