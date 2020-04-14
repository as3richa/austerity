#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include "allocator.h"
#include "common.h"

struct env_op;

#define TYPE_ONLY
#define NAME env_op_vec
#define CONTAINED_TYPE struct env_op
#include "vec.h"

struct austerity_environment {
  graph_builder_t *g;

  char *wd;

  uid_t ruid, euid;
  gid_t rgid, egid;

  unsigned int clearenv : 1;

  env_op_vec_t ops;
};

void destroy_environment(environment_t *env, allocator_t *alc);

int apply_environment(const environment_t *env, errors_t *error);

#endif
