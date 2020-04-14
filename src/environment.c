#include "environment.h"
#include "alloc.h"
#include "common.h"
#include "errors.h"

environment_t *create_environment(graph_builder_t *g) {
  environment_t *env = g_alloc_env(g, __func__);

  if (env == NULL) {
    return NULL;
  }

  *env = (environment_t){g, NULL, 1, (struct un_setenv_op_vec){NULL, 0, 0}, -1, -1, -1, -1};
  return env;
}

void destroy_environment(graph_builder_t *g, environment_t *env) {
  ASSERT(g == env->g);

  g_free(g, env->wd);

  const struct un_setenv_op_vec *ops = &env->un_setenv_ops;

  for (size_t i = 0; i < ops->capacity; i++) {
    g_free(g, ops->ary[i].name);
    g_free(g, ops->ary[i].value);
  }

  g_free(g, ops->ary);
}

int environment_setwd(environment_t *env, const char *path) {
  if (env == NULL) {
    return -1;
  }

  if (path == NULL) {
    record_einval(env->g, __func__, "path is NULL");
    return -1;
  }

  char *my_path = g_copy_str(env->g, path, __func__);

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
  env->un_setenv_ops.size = 0;
  return 0;
}

struct un_setenv_op *emplace_un_setenv_op(environment_t *env, const char *call) {
  struct un_setenv_op_vec *ops = &env->un_setenv_ops;

  ASSERT(ops->size <= ops->capacity);

  if (ops->size == ops->capacity) {
    const size_t capacity = 2 * ops->capacity + 1;
    const size_t elem_size = sizeof(struct un_setenv_op);

    struct un_setenv_op *ary =
        g_realloc(env->g, ops->ary, elem_size, capacity, ops->capacity, call);

    if (ary == NULL) {
      return NULL;
    }

    ops->ary = ary;
    ops->capacity = capacity;
  }

  ASSERT(ops->size < ops->capacity);

  return &ops->ary[ops->size++];
}

int environment_setenv(environment_t *env, const char *name, const char *value, int overwrite) {
  if (env == NULL) {
    return -1;
  }

  graph_builder_t *g = env->g;

  if (name == NULL) {
    record_einval(g, __func__, "name is NULL");
    return -1;
  }

  if (value == NULL) {
    record_einval(g, __func__, "value is NULL");
    return -1;
  }

  char *my_name = g_copy_str(g, name, __func__);

  if (my_name == NULL) {
    return -1;
  }

  char *my_value = g_copy_str(g, value, __func__);

  if (my_value == NULL) {
    g_free(g, my_name);
    return -1;
  }

  struct un_setenv_op *op = emplace_un_setenv_op(env, __func__);

  if (op == NULL) {
    g_free(g, my_name);
    g_free(g, my_value);
    return -1;
  }

  *op = (struct un_setenv_op){1, overwrite, my_name, my_value};
  return 0;
}

int environment_unsetenv(environment_t *env, const char *name) {
  if (env == NULL) {
    return -1;
  }

  graph_builder_t *g = env->g;

  if (name == NULL) {
    record_einval(g, __func__, "name is NULL");
    return -1;
  }

  char *my_name = g_copy_str(g, name, __func__);

  if (my_name == NULL) {
    return -1;
  }

  struct un_setenv_op *op = emplace_un_setenv_op(env, __func__);

  if (op == NULL) {
    g_free(g, my_name);
    return -1;
  }

  *op = (struct un_setenv_op){0, 0, my_name, NULL};
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
