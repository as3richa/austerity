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

struct austerity_program {
  struct {
    stream_processor_t *ary;
    size_t size;
  } sps;
};

// FIXME: destructor

program_t *compile_graph(graph_builder_t *g) {
  if (g == NULL) {
    return NULL;
  }

  allocator_t *alc = &g->alc;
  errors_t *errors = &g->errors;

  graph_t graph;

  if (shallow_copy_graph(&graph, &g->graph, alc, errors) < 0) {
    return NULL;
  }

  stream_vec_t to_drain;
  initialize_stream_vec(&to_drain);

  const size_t orig_sd_size = g->graph.stream_data.size;

  for (stream_t stream = 0; stream < orig_sd_size; stream++) {
    stream_data_t *data = &graph.stream_data.ary[stream];

    switch (data->taps.size) {
    case 0: {
      if (stream_vec_push(&to_drain, stream, alc, errors) < 0) {
        shallow_destroy_graph(&graph, alc);
        destroy_stream_vec(&to_drain, alc);
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
          create_stream_processor(&graph, &in, &stream, 1, &out0, data->taps.size, alc, errors);

      if (junc == NULL) {
        shallow_destroy_graph(&graph, alc);
        destroy_stream_vec(&to_drain, alc);
        a_free(alc, taps);
        return NULL;
      }

      junc->type = SP_SPLITTER;
      junc->u.splitter = (struct sp_splitter){in, out0, n_out};

      for (size_t i = 0; i < n_out; i++) {
        stream_data_t *out_data = &graph.stream_data.ary[out0 + i];

        if (tap_vec_push(&out_data->taps, taps[i], alc, errors) < 0) {
          shallow_destroy_graph(&graph, alc);
          destroy_stream_vec(&to_drain, alc);
          a_free(alc, taps);
          return NULL;
        }
      }

      a_free(alc, taps);

      break;
    }
    }
  }

  if (to_drain.size > 0) {
    tap_t join_in0;
    stream_t joined;
    stream_processor_t *join = create_stream_processor(
        &graph, &join_in0, to_drain.ary, to_drain.size, &joined, 1, alc, errors);

    if (join == NULL) {
      shallow_destroy_graph(&graph, alc);
      destroy_stream_vec(&to_drain, alc);
      return NULL;
    }

    join->type = SP_JOIN;
    join->u.join = (struct sp_join){join_in0, to_drain.size, joined};

    tap_t sink_in;
    stream_processor_t *dev_null =
        create_stream_processor(&graph, &sink_in, &joined, 1, NULL, 0, alc, errors);

    // FIXME: this leaks under esoteric circumstances
    char *path;

    if (dev_null == NULL || (path = a_copy_str(alc, "/dev/null", errors)) == NULL) {
      shallow_destroy_graph(&graph, alc);
      destroy_stream_vec(&to_drain, alc);
      return NULL;
    }

    dev_null->type = SP_PATH_SINK;
    dev_null->u.sink.in = sink_in;
    dev_null->u.sink.u.path = (struct wpath){path, 0};
  }

  ASSERT(graph.stream_data.size == graph.n_taps);

  program_t *prog = a_alloc(alc, sizeof(program_t), errors);

  if (prog == NULL) {
    shallow_destroy_graph(&graph, alc);
    return NULL;
  }

  prog->sps.ary = move_from_stream_processor_vec(&graph.sps, &prog->sps.size);
  return prog;
}
