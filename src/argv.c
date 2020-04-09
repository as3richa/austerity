#include "argv.h"
#include "graph-builder.h"

union argv_arg {
  char *str;
  stream_t *stream;
};

static void destroy_arg(graph_builder_t *g, argv_arg_t *arg);

#define METHODS_ONLY
#define NAME argv_vec
#define CONTAINED_TYPE argv_arg_t
#define DESTRUCTOR destroy_arg
#include "vec.h"

static int push_str(argv_t *argv, const char *arg, const char *api_fn_name);

void initialize_argv(graph_builder_t *g, argv_t *argv) {
  argv->g = g;
  initialize_argv_vec(&argv->args);
}

void destroy_argv(graph_builder_t *g, argv_t *argv) {
  assert(g == argv->g);
  destroy_argv_vec(g, &argv->args);
}

int argv_push_str(argv_t *argv, const char *arg) {
  return push_str(argv, arg, __func__);
}

int argv_push_strs(argv_t *argv, char **const args, size_t n_args) {
  graph_builder_t *g = argv->g;

  NULL_CHECK(g, args, -1);

  for (size_t i = 0; i < n_args; i++) {
    NULL_CHECK(g, args[i], -1);
  }

  argv_arg_t *ary = argv_vec_emplace_n(g, &argv->args, n_args, __func__);

  if (ary == NULL) {
    return -1;
  }

  for (size_t i = 0; i < n_args; i++) {
    char *my_arg = copy_str(g, args[i], __func__);

    if (my_arg == NULL) {
      argv_vec_pop_n(&argv->args, n_args - i);
      return -1;
    }

    ary[i].str = my_arg;
  }

  return 0;
}

int argv_push_strs_v(argv_t *argv, ...) {
  va_list v;
  va_start(v, argv);
  const int result = argv_push_strs_va(argv, v, __func__);
  va_end(v);
  return result;
}

int argv_push_stream(argv_t *argv, stream_t *in);

stream_t *command(graph_builder_t *g, const char *path, argv_t *argv, stream_t *stdin);

stream_t *command_e(stream_t **stderr,
                    graph_builder_t *g,
                    const char *path,
                    argv_t *argv,
                    environment_t *env,
                    stream_t *stdin);

int argv_push_strs_va(austerity_argv_t *argv, va_list v, const char *api_fn_name) {
  for (;;) {
    const char *arg = va_arg(v, const char *);

    if (arg == NULL) {
      return 0;
    }

    if (push_str(argv, arg, api_fn_name) < 0) {
      return -1;
    }
  }
}

static int push_str(argv_t *argv, const char *arg, const char *api_fn_name) {
  graph_builder_t *g = argv->g;

  NULL_CHECK(g, arg, -1);

  char *my_arg = copy_str(g, arg, api_fn_name);

  if (my_arg == NULL) {
    return -1;
  }

  argv_arg_t *dest = argv_vec_emplace(g, &argv->args, api_fn_name);

  if (dest == NULL) {
    ifree(g, my_arg);
    return -1;
  }

  dest->str = my_arg;
  return 0;
}

static void destroy_arg(graph_builder_t *g, argv_arg_t *arg) {
  ifree(g, arg->str);
}
