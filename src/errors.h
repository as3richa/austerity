#ifndef ERRORS_H
#define ERRORS_H

#include "common.h"

struct errors {
  struct austerity_error {
    const char *call;
    int errnum;
    const char *english;
  } first;

  unsigned int abort : 1;

  void (*callback)(void *, graph_builder_t *, const error_t *);
  void *user;
};

void initialize_errors(struct errors *err);

void record_einval(graph_builder_t *g, const char *call, const char *english);
void record_alloc_failure(graph_builder_t *g, const char *call);

#define NULL_CHECK(g, ident, ret, call)                                                            \
  INVAL_CHECK((g), (ident) == NULL, #ident "is NULL", ret, call)

#define INVAL_CHECK(g, cond, message, ret, call)                                                   \
  do {                                                                                             \
    if (cond) {                                                                                    \
      record_einval((g), call, message);                                                           \
      return (ret);                                                                                \
    }                                                                                              \
  } while (0)

#endif
