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

void initialize_errors(errors_t *errors);

void record_einval(errors_t *errors, const char *english);
void record_alloc_failure(errors_t *errors);

#define CHECK_IF_NULL(ident, ret, errors)                                                          \
  CHECK_INVAL((ident) == NULL, #ident "is NULL", ret, errors)

#define CHECK_INVAL(cond, message, ret, errors)                                                    \
  do {                                                                                             \
    if (cond) {                                                                                    \
      record_einval(errors, message);                                                              \
      return (ret);                                                                                \
    }                                                                                              \
  } while (0)

#endif
