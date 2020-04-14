#include "argv.h"
#include "allocator.h"
#include "errors.h"
#include "graph-builder.h"
#include "graph.h"

struct argv_arg {
  enum { ARG_STR, ARG_IN, ARG_OUT } type;

  union {
    char *str;
    tap_t in;
    stream_t out;
  } u;
};

static void destroy_arg(struct argv_arg *arg, allocator_t *alc);

#define METHODS_ONLY
#define NAME argv_vec
#define CONTAINED_TYPE struct argv_arg
#define DESTRUCTOR destroy_arg
#include "vec.h"

static int push_str(argv_t *argv, const char *str, allocator_t *alc, errors_t *errors);
static int push_strs_v(argv_t *argv, va_list v, allocator_t *alc, errors_t *errors);

#define UNUSED_MESSAGE "argv has already been used and cannot be modified"

#define CHECK_IF_USED(argv, ret, errors) CHECK_INVAL((argv)->used, UNUSED_MESSAGE, ret, errors)

argv_t *create_argv(graph_builder_t *g) {
  if (g == NULL) {
    return NULL;
  }

  allocator_t *alc = &g->alc;
  errors_t *errors = &g->errors;

  argv_t *argv = a_alloc_argv(alc, errors);

  if (argv == NULL) {
    return NULL;
  }

  argv->g = g;
  initialize_argv_vec(&argv->args);
  argv->used = 0;

  return argv;
}

argv_t *create_argv_v(graph_builder_t *g, ... /* const char *, ..., NULL */) {
  if (g == NULL) {
    return NULL;
  }

  allocator_t *alc = &g->alc;
  errors_t *errors = &g->errors;

  argv_t *argv = a_alloc_argv(alc, errors);

  if (argv == NULL) {
    return NULL;
  }

  argv->g = g;
  initialize_argv_vec(&argv->args);
  argv->used = 0;

  va_list v;
  va_start(v, g);

  if (push_strs_v(argv, v, alc, errors) < 0) {
    destroy_argv(argv, alc);
    va_end(v);
    return NULL;
  }

  va_end(v);
  return argv;
}

void initialize_argv(argv_t *argv, graph_builder_t *g) {
  argv->g = g;
  initialize_argv_vec(&argv->args);
  argv->used = 0;
}

void destroy_argv(argv_t *argv, allocator_t *alc) {
  destroy_argv_vec(&argv->args, alc);
}

int argv_push_str(argv_t *argv, const char *arg) {
  if (argv == NULL) {
    return -1;
  }

  graph_builder_t *g = argv->g;
  allocator_t *alc = &g->alc;
  errors_t *errors = &g->errors;

  CHECK_IF_USED(argv, -1, errors);
  return push_str(argv, arg, alc, errors);
}

int argv_push_strs(argv_t *argv, char *const *const args, size_t n_args) {
  if (argv == NULL) {
    return -1;
  }

  graph_builder_t *g = argv->g;
  allocator_t *alc = &g->alc;
  errors_t *errors = &g->errors;

  if (n_args > 0) {
    CHECK_IF_NULL(args, -1, errors);

    for (size_t i = 0; i < n_args; i++) {
      CHECK_IF_NULL(args[i], -1, errors);
    }
  }

  if (argv_vec_reserve(&argv->args, n_args, alc, errors) < 0) {
    return -1;
  }

  for (size_t i = 0; i < n_args; i++) {
    char *my_str = a_copy_str(alc, args[i], errors);

    if (my_str == NULL) {
      argv_vec_pop_n(&argv->args, n_args - i, alc);
      return -1;
    }

    struct argv_arg *arg = argv_vec_emplace(&argv->args, alc, errors);
    ASSERT(arg != NULL);

    arg->type = ARG_STR;
    arg->u.str = my_str;
  }

  return 0;
}

int argv_push_strs_v(argv_t *argv, ...) {
  if (argv == NULL) {
    return -1;
  }

  graph_builder_t *g = argv->g;
  allocator_t *alc = &g->alc;
  errors_t *errors = &g->errors;

  CHECK_IF_USED(argv, -1, errors);

  va_list v;
  va_start(v, argv);

  if (push_strs_v(argv, v, alc, errors) < 0) {
    va_end(v);
    return -1;
  }

  va_end(v);
  return 0;
}

int argv_push_input(argv_t *argv, stream_t in) {
  if (argv == NULL) {
    return -1;
  }

  graph_builder_t *g = argv->g;
  allocator_t *alc = &g->alc;
  errors_t *errors = &g->errors;

  CHECK_IF_USED(argv, -1, errors);

  struct argv_arg *arg = argv_vec_emplace(&argv->args, alc, errors);

  if (arg == NULL) {
    return -1;
  }

  tap_t tap = tap_stream(&g->graph, in, alc, errors);

  if (tap == NIL_TAP) {
    argv_vec_unemplace(&argv->args);
    return -1;
  }

  arg->type = ARG_IN;
  arg->u.in = tap;

  argv->has_inputs = 1;

  return 0;
}

stream_t argv_push_output(argv_t *argv) {
  if (argv == NULL) {
    return -1;
  }

  graph_builder_t *g = argv->g;
  allocator_t *alc = &g->alc;
  errors_t *errors = &g->errors;

  CHECK_IF_USED(argv, -1, errors);

  struct argv_arg *arg = argv_vec_emplace(&argv->args, alc, errors);

  if (arg == NULL) {
    return NIL_STREAM;
  }

  const stream_t out = create_stream(&g->graph, alc, errors);

  if (out == NIL_STREAM) {
    argv_vec_unemplace(&argv->args);
    return NIL_STREAM;
  }

  arg->type = ARG_OUT;
  arg->u.out = out;

  argv->has_outputs = 1;

  return out;
}

static int push_str(argv_t *argv, const char *str, allocator_t *alc, errors_t *errors) {
  ASSERT(argv != NULL);

  CHECK_IF_NULL(str, -1, errors);

  char *my_str = a_copy_str(alc, str, errors);

  if (my_str == NULL) {
    return -1;
  }

  struct argv_arg *arg = argv_vec_emplace(&argv->args, alc, errors);

  if (arg == NULL) {
    a_free(alc, my_str);
    return -1;
  }

  arg->type = ARG_STR;
  arg->u.str = my_str;

  return 0;
}

static int push_strs_v(argv_t *argv, va_list v, allocator_t *alc, errors_t *errors) {
  for (;;) {
    const char *arg = va_arg(v, const char *);

    if (arg == NULL) {
      return 0;
    }

    if (push_str(argv, arg, alc, errors) < 0) {
      return -1;
    }
  }
}

static void destroy_arg(struct argv_arg *arg, allocator_t *alc) {
  if (arg->type != ARG_STR) {
    ASSERT(arg->type == ARG_IN || arg->type == ARG_OUT);
    return;
  }

  a_free(alc, arg->u.str);
}
