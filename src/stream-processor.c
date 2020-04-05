#include "stream-processor.h"
#include "graph-builder.h"
#include "vec.h"

struct stream_processor *emplace_stream_processor(tap_t *tap0,
                                                  graph_builder_t *g,
                                                  stream_t **in,
                                                  size_t n_in,
                                                  size_t n_out,
                                                  const char *api_fn_name) {
  for (size_t i = 0; i < n_in; i++) {
    VEC_RESERVE_N(tap_t, g, &in[i]->taps, 1, NULL, api_fn_name);
  }

  struct stream_processor_vec *sps = &g->sps;
  VEC_RESERVE_N(struct stream_processor, g, sps, 1, NULL, api_fn_name);

  stream_t *out = ialloc(g, sizeof(stream_t) * n_out, api_fn_name);

  if (out == NULL) {
    return NULL;
  }

  *tap0 = g->n_taps;

  for (size_t i = 0; i < n_in; i++) {
    struct tap_vec *taps = &in[i]->taps;
    taps->ary[taps->size++] = g->n_taps++;
  }

  struct stream_processor *sp = &sps->ary[sps->size++];
  sp->out = out;

  return sp;
}
