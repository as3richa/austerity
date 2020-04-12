#include "common.h"
#include "errors.h"
#include "graph-builder.h"
#include "graph.h"

static stream_processor_t *emplace_sink(graph_builder_t *g, stream_t in, const char *call);

int fd_sink(graph_builder_t *g, int fd, stream_t in) {
  INVAL_CHECK(g, fd < 0, "fd is negative", -1, __func__);

  stream_processor_t *sp = emplace_sink(g, in, __func__);

  if (sp == NULL) {
    return -1;
  }

  sp->type = SP_FD_SINK;
  sp->u.sink.u.fd = fd;

  return 0;
}

int path_sink(graph_builder_t *g, const char *path, int append, stream_t in) {
  NULL_CHECK(g, path, -1, __func__);

  char *my_path = copy_str(g, path, __func__);

  if (my_path == NULL) {
    return -1;
  }

  stream_processor_t *sp = emplace_sink(g, in, __func__);

  if (sp == NULL) {
    return -1;
  }

  sp->type = SP_PATH_SINK;
  sp->u.sink.u.path = (struct wpath){my_path, !!append};

  return 0;
}

int c_file_sink(graph_builder_t *g, FILE *c_file, stream_t in) {
  NULL_CHECK(g, c_file, -1, __func__);

  stream_processor_t *sp = emplace_sink(g, in, __func__);

  if (sp == NULL) {
    return -1;
  }

  sp->type = SP_C_FILE_SINK;
  sp->u.sink.u.c_file = c_file;

  return 0;
}

static stream_processor_t *emplace_sink(graph_builder_t *g, stream_t in, const char *call) {
  tap_t tap;
  stream_processor_t *sp = create_stream_processor(g, &tap, &in, 1, NULL, 0, call);

  if (sp == NULL) {
    return NULL;
  }

  sp->u.sink.in = tap;
  return sp;
}
