#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include "common.h"

struct austerity_environment {
  graph_builder_t *g;

  char *wd;

  unsigned int clearenv : 1;

  struct un_setenv_op_vec {
    struct un_setenv_op {
      unsigned int set : 1;
      unsigned int overwrite : 1;
      char *name;
      char *value;
    } * ary;

    size_t size;
    size_t capacity;
  } un_setenv_ops;

  uid_t ruid, euid;
  gid_t rgid, egid;
};

void initialize_environment(environment_t *env, graph_builder_t *g);

void destroy_environment(graph_builder_t *g, environment_t *env);

int apply_environment(error_t *err, const environment_t *env);

#endif
