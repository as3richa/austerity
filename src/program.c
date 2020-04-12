#include "common.h"
#include "graph.h"

typedef struct austerity_program {
  struct {
    stream_processor_t *ary;
    size_t size;
  } sps;

  st_size_t n_streams;
  st_size_t n_taps;
  st_size_t fifo_bitmap;
} program_t;

program_t *compile_graph(graph_builder_t *g) {
  if (g == NULL) {
    return NULL;
  }

  struct graph gr;

  if (shallow_copy_graph_from_builder(g, &gr, __func__) < 0) {
    return NULL;
  }

  shallow_destroy_graph(g, &gr);

  return NULL;
}
