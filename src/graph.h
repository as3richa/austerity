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

typedef struct {
  stream_processor_vec_t sps;
  stream_data_vec_t stream_data;
  st_size_t n_taps;
} graph_t;

void initialize_graph(graph_t *graph);
void destroy_graph(graph_t *graph, allocator_t *alc);

int shallow_copy_graph(graph_t *to, graph_t *from, allocator_t *alc, errors_t *errors);
void shallow_destroy_graph(graph_t *graph, allocator_t *alc);

stream_t create_stream(graph_t *graph, allocator_t *alc, errors_t *errors);
tap_t tap_stream(graph_t *graph, stream_t stream, allocator_t *alc, errors_t *errors);

stream_processor_t *create_stream_processor(graph_t *gr,
                                            tap_t *tap0,
                                            const stream_t *in,
                                            size_t n_in,
                                            stream_t *out0,
                                            size_t n_out,
                                            allocator_t *alc,
                                            errors_t *errors);

#endif
