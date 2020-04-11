#ifndef GRAPH_H
#define GRAPH_H

#include "common.h"
#include "stream-processor.h"

#define TYPE_ONLY
#define NAME sp_vec
#define CONTAINED_TYPE stream_processor_t
#include "vec.h"

struct graph {
  sp_vec_t sps;
  tap_t n_taps;
  stream_t dev_null;
};

void initialize_graph(struct graph *gr);
void destroy_graph(graph_builder_t *g, struct graph *gr);

stream_processor_t *emplace_stream_processor(graph_builder_t *g,
                                             tap_t *tap0,
                                             stream_t *const *in,
                                             size_t n_in,
                                             size_t n_out,
                                             const char *call);

#endif
