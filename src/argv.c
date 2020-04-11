#include "argv.h"
#include "alloc.h"
#include "errors.h"
#include "graph-builder.h"
#include "graph.h"

struct argv_arg {
  enum { ARG_STR, ARG_IN, ARG_OUT } type;

  union {
    char *str;
    tap_t in;
    stream_t *out;
  } u;
};

static void destroy_arg(graph_builder_t *g, struct argv_arg *arg);

#define METHODS_ONLY
#define NAME argv_vec
#define CONTAINED_TYPE struct argv_arg
#define DESTRUCTOR destroy_arg
#include "vec.h"

static int push_str(argv_t *argv, const char *arg, const char *call);
static int push_strs_v(argv_t *argv, va_list v, const char *call);

argv_t *create_argv(graph_builder_t *g) {
  argv_t *argv = alloc_argv(g, __func__);

  if (argv == NULL) {
    return NULL;
  }

  argv->g = g;
  initialize_argv_vec(&argv->args);

  return argv;
}

argv_t *create_argv_v(graph_builder_t *g, ... /* const char * */) {
  argv_t *argv = alloc_argv(g, __func__);

  if (argv == NULL) {
    return NULL;
  }

  argv->g = g;
  initialize_argv_vec(&argv->args);

  va_list v;
  va_start(v, g);
  const int result = push_strs_v(argv, v, __func__);
  va_end(v);

  if (result < 0) {
    destroy_argv(g, argv);
    return NULL;
  }

  return argv;
}

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

int argv_push_strs(argv_t *argv, char *const *const args, size_t n_args) {
  graph_builder_t *g = argv->g;

  NULL_CHECK(g, args, -1);

  for (size_t i = 0; i < n_args; i++) {
    NULL_CHECK(g, args[i], -1);
  }

  if (argv_vec_reserve(g, &argv->args, n_args, __func__) < 0) {
    return -1;
  }

  for (size_t i = 0; i < n_args; i++) {
    char *my_str = copy_str(g, args[i], __func__);

    if (my_str == NULL) {
      argv_vec_pop_n(g, &argv->args, n_args - i);
      return -1;
    }

    struct argv_arg *arg = argv_vec_emplace(g, &argv->args, __func__);
    assert(arg != NULL);

    arg->type = ARG_STR;
    arg->u.str = my_str;
  }

  return 0;
}

int argv_push_strs_v(argv_t *argv, ...) {
  va_list v;
  va_start(v, argv);
  const int result = push_strs_v(argv, v, __func__);
  va_end(v);
  return result;
}

int argv_push_input(argv_t *argv, stream_t *in) {
  graph_builder_t *g = argv->g;
  struct graph *gr = &g->gr;

  if (in == NULL) {
    in = &gr->dev_null;
  }

  if (tap_stream(g, in, gr->n_taps, __func__) < 0) {
    return -1;
  }

  struct argv_arg *arg = argv_vec_emplace(g, &argv->args, __func__);

  if (arg == NULL) {
    untap_stream(in);
    return -1;
  }

  arg->type = ARG_IN;
  arg->u.in = gr->n_taps++;

  return 0;
}

stream_t *argv_push_output(argv_t *argv) {
  graph_builder_t *g = argv->g;

  stream_t *out = ialloc(g, sizeof(stream_t), __func__);

  if (out == NULL) {
    return NULL;
  }

  initialize_stream(out);

  struct argv_arg *arg = argv_vec_emplace(g, &argv->args, __func__);

  if (arg == NULL) {
    destroy_stream(g, out);
    ifree(g, out);
    return NULL;
  }

  arg->type = ARG_OUT;
  arg->u.out = out;

  return out;
}

static int push_strs_v(argv_t *argv, va_list v, const char *call) {
  for (;;) {
    const char *arg = va_arg(v, const char *);

    if (arg == NULL) {
      return 0;
    }

    if (push_str(argv, arg, call) < 0) {
      return -1;
    }
  }
}

static int push_str(argv_t *argv, const char *str, const char *call) {
  graph_builder_t *g = argv->g;

  NULL_CHECK(g, str, -1);

  char *my_str = copy_str(g, str, call);

  if (my_str == NULL) {
    return -1;
  }

  struct argv_arg *arg = argv_vec_emplace(g, &argv->args, call);

  if (arg == NULL) {
    ifree(g, my_str);
    return -1;
  }

  arg->type = ARG_STR;
  arg->u.str = my_str;

  return 0;
}

static void destroy_arg(graph_builder_t *g, struct argv_arg *arg) {
  switch (arg->type) {
  case ARG_STR:
    ifree(g, arg->u.str);
    break;

  case ARG_IN:
    break;

  case ARG_OUT:
    destroy_stream(g, arg->u.out);
    ifree(g, arg->u.out);
    break;

  default:
    assert(0);
  }
}
