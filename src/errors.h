#ifndef ERRORS_H
#define ERRORS_H

#include "common.h"

typedef struct {
  struct austerity_error {
    const char *call;
    int errnum;
    const char *english;
  } first;

  unsigned int abort : 1;

  void (*callback)(void *, const error_t *);
  void *user;
} errors_t;

void initialize_errors(errors_t *es);

void record_einval(errors_t *es, const char *call, const char *english);
void record_alloc_failure(errors_t *es, const char *call);

#endif
