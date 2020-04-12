#include "common.h"
#include "graph-builder.h"
#include "graph.h"

// Does this duplication matter? Should I just put the methods in a header? FIXME
#define METHODS_ONLY
#define NAME stream_processor_vec
#define CONTAINED_TYPE stream_processor_t
#include "vec.h"

#define METHODS_ONLY
#define NAME tap_vec
#define CONTAINED_TYPE tap_t
#include "vec.h"

#define NAME stream_vec
#define CONTAINED_TYPE stream_t
#include "vec.h"

typedef struct austerity_program {
  struct {
    stream_processor_t *ary;
    size_t size;
  } sps;
} program_t;

program_t *compile_graph(graph_builder_t *g) {
  if (g == NULL) {
    return NULL;
  }

  struct graph gr;

  if (shallow_copy_graph_from_builder(g, &gr, __func__) < 0) {
    return NULL;
  }

  stream_vec_t to_drain;
  initialize_stream_vec(&to_drain);

  const size_t orig_sd_size = g->gr.stream_data.size;

  for (stream_t stream = 0; stream < orig_sd_size; stream++) {
    stream_data_t *data = &gr.stream_data.ary[stream];

    switch (data->taps.size) {
    case 0: {
      if (stream_vec_push(g, &to_drain, stream, __func__) < 0) {
        shallow_destroy_graph(g, &gr);
        destroy_stream_vec(g, &to_drain);
        return NULL;
      }

      break;
    }

    case 1: {
      break;
    }

    default: {
      size_t n_out;
      tap_t *taps = move_from_tap_vec(&data->taps, &n_out);

      tap_t in;
      stream_t out0;
      stream_processor_t *junc =
          create_stream_processor(g, &gr, &in, &stream, 1, &out0, data->taps.size, __func__);

      if (junc == NULL) {
        shallow_destroy_graph(g, &gr);
        destroy_stream_vec(g, &to_drain);
        ifree(g, taps);
        return NULL;
      }

      junc->type = SP_SPLITTER;
      junc->u.splitter = (struct sp_splitter){in, out0, n_out};

      for (size_t i = 0; i < n_out; i++) {
        stream_data_t *out_data = &gr.stream_data.ary[out0 + i];

        if (tap_vec_push(g, &out_data->taps, taps[i], __func__) < 0) {
          shallow_destroy_graph(g, &gr);
          destroy_stream_vec(g, &to_drain);
          ifree(g, taps);
          return NULL;
        }
      }

      ifree(g, taps);

      break;
    }
    }
  }

  shallow_destroy_graph(g, &gr);
  destroy_stream_vec(g, &to_drain);

  return NULL;
}
