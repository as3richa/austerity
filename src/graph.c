#include "func.h"
#include "graph-builder.h"
#include "stream-processor.h"
#include "stream.h"

static void destroy_stream_processor(graph_builder_t *g, stream_processor_t *sp);

#define METHODS_ONLY
#define NAME sp_vec
#define CONTAINED_TYPE stream_processor_t
#define DESTRUCTOR destroy_stream_processor
#include "vec.h"

void initialize_graph(struct graph *gr) {
  initialize_sp_vec(&gr->sps);
  gr->n_taps = 0;
  initialize_stream(&gr->dev_null);
}

void destroy_graph(graph_builder_t *g, struct graph *gr) {
  destroy_sp_vec(g, &gr->sps);
  destroy_stream(g, &gr->dev_null);
}

stream_processor_t *emplace_stream_processor(graph_builder_t *g,
                                             tap_t *tap0,
                                             stream_t *const *in,
                                             size_t n_in,
                                             size_t n_out,
                                             const char *call) {
  struct graph *gr = &g->gr;

  stream_t *out = ialloc(g, sizeof(stream_t) * n_out, call);

  if (out == NULL) {
    return NULL;
  }

  for (size_t i = 0; i < n_out; i++) {
    initialize_stream(&out[i]);
  }

  stream_processor_t *sp = sp_vec_emplace(g, &gr->sps, call);

  if (sp == NULL) {
    for (size_t i = 0; i < n_out; i++) {
      destroy_stream(g, &out[i]);
    }
    ifree(g, out);
    return NULL;
  }

  for (size_t i = 0; i < n_in; i++) {
    stream_t *stream = (in[i] == NULL) ? &gr->dev_null : in[i];

    if (tap_stream(g, stream, gr->n_taps + i, call) < 0) {
      for (size_t i = 0; i < n_out; i++) {
        destroy_stream(g, &out[i]);
      }
      ifree(g, out);

      for (size_t j = 0; j < i; j++) {
        stream_t *tapped_stream = (in[i] == NULL) ? &gr->dev_null : in[i];
        untap_stream(tapped_stream);
      }

      return NULL;
    }
  }

  if (n_in > 0) {
    *tap0 = gr->n_taps;
    gr->n_taps += n_in;
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

  case SP_FUNCTION:
    n_out = sp->u.function.func->n_out;
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
    destroy_stream(g, &sp->out[i]);
  }
  ifree(g, sp->out);
}
