#include "common.h"
#include "func.h"
#include "graph-builder.h"
#include "graph.h"

#define NAME stream_vec
#define CONTAINED_TYPE stream_t
#include "vec.h"

static int my_function_e(graph_builder_t *g,
                         stream_t *out,
                         environment_t *env,
                         func_t *func,
                         void *user,
                         const stream_t *in,
                         size_t n_in,
                         const char *call);

int function(
    graph_builder_t *g, stream_t *out, func_t *func, void *user, const stream_t *in, size_t n_in) {
  return my_function_e(g, out, NULL, func, user, in, n_in, __func__);
}

int function_0(graph_builder_t *g, stream_t *out, func_t *func, void *user) {
  return my_function_e(g, out, NULL, func, user, NULL, 0, __func__);
}

int function_1(graph_builder_t *g, stream_t *out, func_t *func, void *user, stream_t in) {
  return my_function_e(g, out, NULL, func, user, &in, 1, __func__);
}

int function_2(
    graph_builder_t *g, stream_t *out, func_t *func, void *user, stream_t left, stream_t right) {
  stream_t in[2] = {left, right};
  return my_function_e(g, out, NULL, func, user, in, 2, __func__);
}

int function_e(graph_builder_t *g,
               stream_t *out,
               environment_t *env,
               func_t *func,
               void *user,
               const stream_t *in,
               size_t n_in) {
  return my_function_e(g, out, env, func, user, in, n_in, __func__);
}

int function_v(graph_builder_t *g,
               stream_t *out,
               func_t *func,
               void *user,
               ... /* stream_t, ..., STREAM_VA_END */) {
  stream_vec_t in;
  initialize_stream_vec(&in);

  va_list v;
  va_start(v, user);

  for (;;) {
    stream_t stream = va_arg(v, stream_t);

    if (stream == STREAM_VA_END) {
      break;
    }

    if (stream_vec_push(g, &in, stream, __func__) < 0) {
      destroy_stream_vec(g, &in);
      va_end(v);
      return -1;
    }
  }

  va_end(v);

  const int result = my_function_e(g, out, NULL, func, user, in.ary, in.size, __func__);
  destroy_stream_vec(g, &in);

  return result;
}

static int my_function_e(graph_builder_t *g,
                         stream_t *out,
                         environment_t *env,
                         func_t *func,
                         void *user,
                         const stream_t *in,
                         size_t n_in,
                         const char *call) {
  if (env == NULL) {
    env = g->default_env;
  }

  const size_t n_out = func->n_out;

  tap_t tap0;
  stream_t out0;
  stream_processor_t *sp = create_stream_processor(g, &tap0, in, n_in, &out0, n_out, call);

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
