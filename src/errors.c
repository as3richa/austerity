#include "errors.h"
#include "graph-builder.h"

static void record_error(graph_builder_t *g, const char *call, int errnum, const char *english);

void initialize_errors(struct errors *err) {
  *err = (struct errors){{NULL, 0, ""}, 0, NULL, NULL};
}

void graph_builder_abort_on_error(graph_builder_t *g) {
  g->err.abort = 1;
}

void graph_builder_on_error(graph_builder_t *g,
                            void (*callback)(void *, graph_builder_t *, const error_t *),
                            void *user) {
  struct errors *err = &g->err;
  err->callback = callback;
  err->user = user;
}

void record_einval(graph_builder_t *g, const char *call, const char *english) {
  record_error(g, call, EINVAL, english);
}

void record_alloc_failure(graph_builder_t *g, const char *call) {
  record_error(g, call, (errno == 0) ? -1 : errno, "cannot allocate memory");
}

static void record_error(graph_builder_t *g, const char *call, int errnum, const char *english) {
  struct errors *err = &g->err;

  const error_t error = {call, (errnum == 0) ? -1 : errnum, english};

  if (err->first.errnum == 0) {
    err->first = error;
  }

  if (err->callback != NULL) {
    (*err->callback)(err->user, g, &error);
  }

  if (err->abort) {
    fprintf(stderr, "austerity: %s: %s (errno %d); aborting\n", call, english, errnum);
    fflush(stderr);
    abort();
  }
}
