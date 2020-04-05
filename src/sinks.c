#include "common.h"
#include "graph-builder.h"
#include "stream-processor.h"

static struct stream_processor *
emplace_sink(graph_builder_t *g, stream_t *in, const char *api_fn_name);

int fd_sink(graph_builder_t *g, int fd, stream_t *in) {
  INVAL_CHECK(g, fd < 0, "fd is negative", -1);

  struct stream_processor *sp = emplace_sink(g, in, __func__);

  if (sp == NULL) {
    return -1;
  }

  sp->type = SP_FD_SINK;
  sp->u.sink.u.fd = fd;

  return 0;
}

int path_sink(graph_builder_t *g, const char *path, int append, stream_t *in) {
  NULL_CHECK(g, path, -1);

  char *my_path = copy_str(g, path, __func__);

  if (my_path == NULL) {
    return -1;
  }

  struct stream_processor *sp = emplace_sink(g, in, __func__);

  if (sp == NULL) {
    return -1;
  }

  sp->type = SP_PATH_SINK;
  sp->u.sink.u.path = (struct wpath){my_path, !!append};

  return 0;
}

int c_file_sink(graph_builder_t *g, FILE *c_file, stream_t *in) {
  NULL_CHECK(g, c_file, -1);

  struct stream_processor *sp = emplace_sink(g, in, __func__);

  if (sp == NULL) {
    return -1;
  }

  sp->u.sink.u.c_file = c_file;

  return 0;
}

static struct stream_processor *
emplace_sink(graph_builder_t *g, stream_t *in, const char *api_fn_name) {
  tap_t tap;
  struct stream_processor *sp = emplace_stream_processor(&tap, g, &in, 1, 0, api_fn_name);

  if (sp == NULL) {
    return NULL;
  }

  sp->u.sink.in = tap;
  return sp;
}
