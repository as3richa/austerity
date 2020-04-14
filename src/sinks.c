#include "common.h"
#include "errors.h"
#include "graph-builder.h"
#include "graph.h"

static stream_processor_t *
emplace_sink(graph_t *graph, stream_t in, allocator_t *alc, errors_t *errors);

int fd_sink(graph_builder_t *g, int fd, stream_t in) {
  allocator_t *alc = &g->alc;
  errors_t *errors = &g->errors;

  CHECK_INVAL(fd < 0, "fd is negative", -1, errors);

  stream_processor_t *sp = emplace_sink(&g->graph, in, alc, errors);

  if (sp == NULL) {
    return -1;
  }

  sp->type = SP_FD_SINK;
  sp->u.sink.u.fd = fd;

  return 0;
}

int path_sink(graph_builder_t *g, const char *path, int append, stream_t in) {
  allocator_t *alc = &g->alc;
  errors_t *errors = &g->errors;

  CHECK_IF_NULL(path, -1, errors);

  char *my_path = a_copy_str(alc, path, errors);

  if (my_path == NULL) {
    return -1;
  }

  stream_processor_t *sp = emplace_sink(&g->graph, in, alc, errors);

  if (sp == NULL) {
    return -1;
  }

  sp->type = SP_PATH_SINK;
  sp->u.sink.u.path = (struct wpath){my_path, !!append};

  return 0;
}

int c_file_sink(graph_builder_t *g, FILE *c_file, stream_t in) {
  allocator_t *alc = &g->alc;
  errors_t *errors = &g->errors;

  CHECK_IF_NULL(c_file, -1, errors);

  stream_processor_t *sp = emplace_sink(&g->graph, in, alc, errors);

  if (sp == NULL) {
    return -1;
  }

  sp->type = SP_C_FILE_SINK;
  sp->u.sink.u.c_file = c_file;

  return 0;
}

static stream_processor_t *
emplace_sink(graph_t *graph, stream_t in, allocator_t *alc, errors_t *errors) {
  tap_t tap;
  stream_processor_t *sp = create_stream_processor(graph, &tap, &in, 1, NULL, 0, alc, errors);

  if (sp == NULL) {
    return NULL;
  }

  sp->u.sink.in = tap;
  return sp;
}
