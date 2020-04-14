#include "environment.h"
#include "common.h"
#include "errors.h"
#include "graph-builder.h"

struct env_op {
  unsigned int set : 1;
  unsigned int overwrite : 1;
  char *name;
  char *value;
};

static void destroy_env_op(struct env_op *op, allocator_t *alc);

#define METHODS_ONLY
#define NAME env_op_vec
#define CONTAINED_TYPE struct env_op
#define DESTRUCTOR destroy_env_op
#include "vec.h"

environment_t *create_environment(graph_builder_t *g) {
  allocator_t *alc = &g->alc;
  errors_t *errors = &g->errors;

  environment_t *env = a_alloc_env(alc, errors);

  if (env == NULL) {
    return NULL;
  }

  env->g = g;
  env->wd = NULL;
  env->ruid = -1;
  env->euid = -1;
  env->rgid = -1;
  env->egid = -1;
  env->clearenv = 1;
  initialize_env_op_vec(&env->ops);

  return env;
}

void destroy_environment(environment_t *env, allocator_t *alc) {
  if (env == NULL) {
    return;
  }

  a_free(alc, env->wd);
  destroy_env_op_vec(&env->ops, alc);
}

int environment_setwd(environment_t *env, const char *path) {
  if (env == NULL) {
    return -1;
  }

  graph_builder_t *g = env->g;
  allocator_t *alc = &g->alc;
  errors_t *errors = &g->errors;

  CHECK_IF_NULL(path, -1, errors);

  char *my_path = a_copy_str(alc, path, errors);

  if (my_path == NULL) {
    return -1;
  }

  env->wd = my_path;
  return 0;
}

int environment_preserve_env(environment_t *env) {
  if (env == NULL) {
    return -1;
  }

  env->clearenv = 0;
  return 0;
}

int environment_clearenv(environment_t *env) {
  if (env == NULL) {
    return -1;
  }

  env->clearenv = 1;
  clear_env_op_vec(&env->ops);

  return 0;
}

int environment_setenv(environment_t *env, const char *name, const char *value, int overwrite) {
  if (env == NULL) {
    return -1;
  }

  graph_builder_t *g = env->g;
  allocator_t *alc = &g->alc;
  errors_t *errors = &g->errors;

  CHECK_IF_NULL(name, -1, errors);
  CHECK_IF_NULL(value, -1, errors);

  char *my_name = a_copy_str(alc, name, errors);

  if (my_name == NULL) {
    return -1;
  }

  char *my_value = a_copy_str(alc, value, errors);

  if (my_value == NULL) {
    a_free(alc, my_name);
    return -1;
  }

  struct env_op *op = env_op_vec_emplace(&env->ops, alc, errors);

  if (op == NULL) {
    a_free(alc, my_name);
    a_free(alc, my_value);
    return -1;
  }

  op->set = 1;
  op->overwrite = overwrite;
  op->name = my_name;
  op->value = my_value;

  return 0;
}

int environment_unsetenv(environment_t *env, const char *name) {
  if (env == NULL) {
    return -1;
  }

  graph_builder_t *g = env->g;
  allocator_t *alc = &g->alc;
  errors_t *errors = &g->errors;

  CHECK_IF_NULL(name, -1, errors);

  char *my_name = a_copy_str(alc, name, errors);

  if (my_name == NULL) {
    return -1;
  }

  struct env_op *op = env_op_vec_emplace(&env->ops, alc, errors);

  if (op == NULL) {
    a_free(alc, my_name);
    return -1;
  }

  op->overwrite = 0;
  op->name = my_name;
  op->value = NULL;

  return 0;
}

int environment_setreuid(environment_t *env, uid_t ruid, uid_t euid) {
  if (env == NULL) {
    return -1;
  }

  env->ruid = ruid;
  env->euid = euid;

  return 0;
}

int environment_setregid(environment_t *env, gid_t rgid, gid_t egid) {
  if (env == NULL) {
    return -1;
  }

  env->rgid = rgid;
  env->egid = egid;

  return 0;
}

static void destroy_env_op(struct env_op *op, allocator_t *alc) {
  a_free(alc, op->name);
  a_free(alc, op->value);
}
