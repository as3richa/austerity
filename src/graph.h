#ifndef GRAPH_H
#define GRAPH_H

#include "common.h"
#include "stream-processor.h"

#define TYPE_ONLY
#define NAME stream_processor_vec
#define CONTAINED_TYPE stream_processor_t
#include "vec.h"

#define TYPE_ONLY
#define NAME tap_vec
#define CONTAINED_TYPE tap_t
#include "vec.h"

typedef struct {
  tap_vec_t taps;
} stream_data_t;

#define TYPE_ONLY
#define NAME stream_data_vec
#define CONTAINED_TYPE stream_data_t
#include "vec.h"

struct graph {
  stream_processor_vec_t sps;
  stream_data_vec_t stream_data;
  st_size_t n_taps;
};

void initialize_graph(struct graph *gr);
void destroy_graph(graph_builder_t *g, struct graph *gr);

stream_t create_stream(graph_builder_t *g, const char *call);
tap_t tap_stream(graph_builder_t *g, stream_t stream, const char *call);

stream_processor_t *create_stream_processor(graph_builder_t *g,
                                            tap_t *tap0,
                                            const stream_t *in,
                                            size_t n_in,
                                            stream_t *out0,
                                            size_t n_out,
                                            const char *call);

#endif
