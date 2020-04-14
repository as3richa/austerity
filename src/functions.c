#include "common.h"
#include "func.h"
#include "graph-builder.h"
#include "graph.h"

#define NAME stream_vec
#define CONTAINED_TYPE stream_t
#include "vec.h"

static int my_function_e(graph_t *graph,
                         stream_t *out,
                         environment_t *env,
                         func_t *func,
                         void *user,
                         const stream_t *in,
                         size_t n_in,
                         allocator_t *alc,
                         errors_t *errors);

int function(
    graph_builder_t *g, stream_t *out, func_t *func, void *user, const stream_t *in, size_t n_in) {
  return my_function_e(&g->graph, out, g->default_env, func, user, in, n_in, &g->alc, &g->errors);
}

int function_0(graph_builder_t *g, stream_t *out, func_t *func, void *user) {
  return my_function_e(&g->graph, out, g->default_env, func, user, NULL, 0, &g->alc, &g->errors);
}

int function_1(graph_builder_t *g, stream_t *out, func_t *func, void *user, stream_t in) {
  return my_function_e(&g->graph, out, g->default_env, func, user, &in, 1, &g->alc, &g->errors);
}

int function_2(
    graph_builder_t *g, stream_t *out, func_t *func, void *user, stream_t left, stream_t right) {
  stream_t in[2] = {left, right};
  return my_function_e(&g->graph, out, g->default_env, func, user, in, 2, &g->alc, &g->errors);
}

int function_e(graph_builder_t *g,
               stream_t *out,
               environment_t *env,
               func_t *func,
               void *user,
               const stream_t *in,
               size_t n_in) {
  if (env == NULL) {
    env = g->default_env;
  }

  return my_function_e(&g->graph, out, env, func, user, in, n_in, &g->alc, &g->errors);
}

int function_v(graph_builder_t *g,
               stream_t *out,
               func_t *func,
               void *user,
               ... /* stream_t, ..., STREAM_VA_END */) {
  allocator_t *alc = &g->alc;
  errors_t *errors = &g->errors;

  stream_vec_t in;
  initialize_stream_vec(&in);

  va_list v;
  va_start(v, user);

  for (;;) {
    stream_t stream = va_arg(v, stream_t);

    if (stream == STREAM_VA_END) {
      break;
    }

    if (stream_vec_push(&in, stream, alc, errors) < 0) {
      destroy_stream_vec(&in, alc);
      va_end(v);
      return -1;
    }
  }

  va_end(v);

  const int result =
      my_function_e(&g->graph, out, g->default_env, func, user, in.ary, in.size, alc, errors);
  destroy_stream_vec(&in, alc);

  return result;
}

static int my_function_e(graph_t *graph,
                         stream_t *out,
                         environment_t *env,
                         func_t *func,
                         void *user,
                         const stream_t *in,
                         size_t n_in,
                         allocator_t *alc,
                         errors_t *errors) {
  const size_t n_out = func->n_out;

  tap_t tap0;
  stream_t out0;
  stream_processor_t *sp =
      create_stream_processor(graph, &tap0, in, n_in, &out0, n_out, alc, errors);

  if (sp == NULL) {
    return -1;
  }

  sp->type = SP_FUNCTION;
  sp->u.function = (struct sp_function){env, func, user, tap0, n_in, out0, n_out};

  for (size_t i = 0; i < n_out; i++) {
    out[i] = out0 + i;
  }

  return 0;
}
