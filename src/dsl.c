#include "dsl.h"
#include "graph-builder.h"

#define METHODS_ONLY
#define NAME tap_vec
#define CONTAINED_TYPE tap_t
#include "vec.h"

static void destroy_stream_processor(graph_builder_t *g, stream_processor_t *sp);

#define METHODS_ONLY
#define NAME sp_vec
#define CONTAINED_TYPE stream_processor_t
#define DESTRUCTOR destroy_stream_processor
#include "vec.h"

void initialize_dsl_state(dsl_state_t *dsl) {
  initialize_sp_vec(&dsl->sps);
  dsl->n_taps = 0;
}

void destroy_dsl_state(graph_builder_t *g, dsl_state_t *dsl) {
  destroy_sp_vec(g, &dsl->sps);
}

stream_processor_t *emplace_stream_processor(graph_builder_t *g,
                                             tap_t *tap0,
                                             stream_t *const *in,
                                             size_t n_in,
                                             size_t n_out,
                                             const char *call) {
  dsl_state_t *dsl = &g->dsl;

  stream_t *out = ialloc(g, sizeof(stream_t) * n_out, call);

  if (out == NULL) {
    return NULL;
  }

  for (size_t i = 0; i < n_out; i++) {
    initialize_tap_vec(&out[i].taps);
  }

  stream_processor_t *sp = sp_vec_emplace(g, &dsl->sps, call);

  if (sp == NULL) {
    ifree(g, out);
    return NULL;
  }

  for (size_t i = 0; i < n_in; i++) {
    if (tap_vec_push(g, &in[i]->taps, dsl->n_taps + i, call) < 0) {
      // FIXME: ???
      return NULL;
    }
  }

  if (n_in != 0) {
    *tap0 = dsl->n_taps;
    dsl->n_taps += n_in;
  }

  sp->out = out;
  return sp;
}

static void destroy_stream_processor(graph_builder_t *g, stream_processor_t *sp) {
  size_t n_out;

  switch (sp->type) {
  case SP_PATH_SOURCE:
    ifree(g, sp->u.source.path);
    n_out = 1;
    break;

  case SP_STR_SOURCE:
  case SP_BUFFER_SOURCE:
    ifree(g, sp->u.source.buf.bytes);
    n_out = 1;
    break;

  case SP_PATH_SINK:
    ifree(g, sp->u.sink.u.path.path);
    n_out = 1;
    break;

  case SP_COMMAND:
    ifree(g, sp->u.command.path);
    n_out = 2;
    break;

  case SP_FD_SOURCE:
  case SP_C_FILE_SOURCE:
  case SP_STATIC_STR_SOURCE:
  case SP_STATIC_BUFFER_SOURCE:
    n_out = 1;
    break;

  case SP_FD_SINK:
  case SP_C_FILE_SINK:
    n_out = 0;
    break;

  default:
    assert(0);
    n_out = -1;
  }

  for (size_t i = 0; i < n_out; i++) {
    destroy_tap_vec(g, &sp->out[i].taps);
  }

  ifree(g, sp->out);
}
