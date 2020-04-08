#include "dsl.h"
#include "graph-builder.h"

void initialize_dsl_state(dsl_state_t *dsl) {
  initialize_sp_vec(&dsl->sps);
}

void destroy_dsl_state(graph_builder_t *g, dsl_state_t *dsl) {
  destroy_sp_vec(g, &dsl->sps);
}

stream_processor_t *emplace_stream_processor(graph_builder_t *g,
                                             tap_t *tap0,
                                             stream_t **in,
                                             size_t n_in,
                                             size_t n_out,
                                             const char *api_fn_name) {
  dsl_state_t *dsl = graph_builder_dsl(g);

  stream_t *out = ialloc(g, sizeof(stream_t) * n_out, api_fn_name);

  if (out == NULL) {
    return NULL;
  }

  stream_processor_t *sp = sp_vec_emplace(g, &dsl->sps, api_fn_name);

  if (sp == NULL) {
    ifree(g, out);
    return NULL;
  }

  for (size_t i = 0; i < n_in; i++) {
    if (tap_vec_push(g, &in[i]->taps, dsl->n_taps + i, api_fn_name) < 0) {
      ifree(g, out);
      sp_vec_pop(&dsl->sps);

      while (i--) {
        tap_vec_pop(&in[i]->taps);
      }

      // FIXME: ???
      return NULL;
    }
  }

  *tap0 = dsl->n_taps;
  dsl->n_taps += n_in;

  sp->out = out;
  return sp;
}
