#include "errors.h"
#include "graph-builder.h"

static void record_error(errors_t *es, const char *call, int errnum, const char *english);

void initialize_errors(errors_t *errors) {
  *errors = (errors_t){{NULL, 0, ""}, 0, NULL, NULL};
}

void graph_builder_abort_on_error(graph_builder_t *g) {
  g->errors.abort = 1;
}

void graph_builder_on_error(graph_builder_t *g,
                            void (*callback)(void *, graph_builder_t *, const error_t *),
                            void *user) {
  errors_t *errors = &g->errors;
  errors->callback = callback;
  errors->user = user;
}

void record_einval(errors_t *es, const char *call, const char *english) {
  record_error(es, call, EINVAL, english);
}

void record_alloc_failure(errors_t *es, const char *call) {
  record_error(es, call, (errno == 0) ? -1 : errno, "cannot allocate memory");
}

static void record_error(errors_t *es, const char *call, int errnum, const char *english) {
  const error_t error = {call, (errnum == 0) ? -1 : errnum, english};

  if (es->first.errnum == 0) {
    es->first = error;
  }

  if (es->callback != NULL) {
    (*es->callback)(es->user, &error);
  }

  if (es->abort) {
    fprintf(stderr, "austerity: %s: %s (errno %d); aborting\n", call, english, errnum);
    fflush(stderr);
    abort();
  }
}
