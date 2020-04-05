#include "commands.h"
#include "graph-builder.h"
#include "stream-processor.h"
#include "vec.h"

static int push_str(argv_t *argv, const char *arg, const char *api_fn_name);

void initialize_argv(argv_t *argv, graph_builder_t *g) {
  *argv = (argv_t){g, {NULL, 0, 0}, {NULL, 0, 0}};
}

void destroy_argv(argv_t *argv) {
  for (size_t i = 0; i < argv->args.size; i++) {
    // FIXME: don't free streams :)
    ifree(argv->g, argv->args.ary[i].str);
  }

  ifree(argv->g, argv->args.ary);
}

int argv_push_str(argv_t *argv, const char *arg) {
  return push_str(argv, arg, __func__);
}

int argv_push_strs(argv_t *argv, char **const args, size_t n_args) {
  NULL_CHECK(argv->g, args, -1);

  VEC_RESERVE_N(union arg, argv->g, &argv->args, n_args, -1, __func__);

  for (size_t i = 0; i < n_args; i++) {
    const char *arg = args[i];
    NULL_CHECK(argv->g, arg, -1); // FIXME: this leaks memory :(

    char *my_arg = copy_str(argv->g, arg, __func__);

    if (my_arg == NULL) {
      while (i--) {
        ifree(argv->g, argv->args.ary[--argv->args.size].str);
      }

      return -1;
    }

    argv->args.ary[argv->args.size++].str = my_arg;
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
  NULL_CHECK(argv->g, arg, -1);

  char *my_arg = copy_str(argv->g, arg, api_fn_name);

  if (my_arg == NULL) {
    return -1;
  }

  VEC_RESERVE_N(union arg, argv->g, &argv->args, 1, -1, api_fn_name);

  union arg *item = &argv->args.ary[argv->args.size++];
  item->str = my_arg;

  return 0;
}
