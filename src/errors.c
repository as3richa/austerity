#include "errors.h"
#include "graph-builder.h"

static void record_error(errors_t *errors, int errnum, const char *english);

void initialize_errors(errors_t *errors) {
  errors->first = (error_t){NULL, 0, NULL};
  errors->abort = 0;
  errors->callback = NULL;
  errors->user = NULL;
}

void graph_builder_abort_on_error(graph_builder_t *g) {
  g->errors.abort = 1;
}

void graph_builder_on_error(graph_builder_t *g,
                            void (*callback)(void *, const error_t *),
                            void *user) {
  errors_t *errors = &g->errors;
  errors->callback = callback;
  errors->user = user;
}

void record_einval(errors_t *errors, const char *english) {
  record_error(errors, EINVAL, english);
}

void record_alloc_failure(errors_t *errors) {
  record_error(errors, (errno == 0) ? -1 : errno, "cannot allocate memory");
}

static void record_error(errors_t *errors, int errnum, const char *english) {
  // FIXME: record call, somewhere
  const error_t error = {NULL, (errnum == 0) ? -1 : errnum, english};

  if (errors->first.errnum == 0) {
    errors->first = error;
  }

  if (errors->callback != NULL) {
    (*errors->callback)(errors->user, &error);
  }

  if (errors->abort) {
    fprintf(stderr, "austerity: %s: %s (errno %d); aborting\n", "", english, errnum);
    fflush(stderr);
    abort();
  }
}
