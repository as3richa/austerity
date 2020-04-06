#include "dsl.h"
#include "graph-builder.h"

stream_processor_t *emplace_stream_processor(tap_t *tap0,
                                             graph_builder_t *g,
                                             stream_t **in,
                                             size_t n_in,
                                             size_t n_out,
                                             const char *api_fn_name) {
  stream_t *out = ialloc(g, sizeof(stream_t) * n_out, api_fn_name);

  if (out == NULL) {
    return NULL;
  }

  stream_processor_t *sp = stream_processor_vec_emplace(g, &g->sps, api_fn_name);

  if (sp == NULL) {
    ifree(g, out);
    return NULL;
  }

  for (size_t i = 0; i < n_in; i++) {
    if (tap_vec_push(g, &in[i]->taps, g->n_taps + i, api_fn_name) < 0) {
      ifree(g, out);
      stream_processor_vec_pop(&g->sps);

      while (i--) {
        tap_vec_pop(&in[i]->taps);
      }
    }
  }

  *tap0 = g->n_taps;
  g->n_taps += n_in;

  sp->out = out;
  return sp;
}
