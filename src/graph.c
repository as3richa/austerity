#include "graph.h"
#include "graph-builder.h"
#include "stream-processor.h"

static void destroy_stream_processor(stream_processor_t *sp, allocator_t *alc);

#define METHODS_ONLY
#define NAME stream_processor_vec
#define CONTAINED_TYPE stream_processor_t
#define DESTRUCTOR destroy_stream_processor
#include "vec.h"

#define METHODS_ONLY
#define NAME tap_vec
#define CONTAINED_TYPE tap_t
#include "vec.h"

static void destroy_stream_data(stream_data_t *sd, allocator_t *alc);

#define METHODS_ONLY
#define NAME stream_data_vec
#define CONTAINED_TYPE stream_data_t
#define DESTRUCTOR destroy_stream_data
#include "vec.h"

static stream_t
create_streams(stream_data_vec_t *stream_data, size_t n, allocator_t *alc, errors_t *errors);

static void uncreate_streams(stream_data_vec_t *stream_data, size_t n, allocator_t *alc);

void initialize_graph(graph_t *graph) {
  initialize_stream_processor_vec(&graph->sps);
  initialize_stream_data_vec(&graph->stream_data);
  graph->n_taps = 0;
}

void destroy_graph(graph_t *graph, allocator_t *alc) {
  destroy_stream_processor_vec(&graph->sps, alc);
  destroy_stream_data_vec(&graph->stream_data, alc);
}

int shallow_copy_graph(graph_t *to, graph_t *from, allocator_t *alc, errors_t *errors) {
  if (shallow_copy_stream_processor_vec(&to->sps, &from->sps, alc, errors) < 0) {
    return -1;
  }

  if (shallow_copy_stream_data_vec(&to->stream_data, &from->stream_data, alc, errors) < 0) {
    shallow_destroy_stream_processor_vec(&to->sps, alc);
    return -1;
  }

  to->n_taps = from->n_taps;
  return 0;
}

void shallow_destroy_graph(graph_t *graph, allocator_t *alc) {
  shallow_destroy_stream_processor_vec(&graph->sps, alc);
  shallow_destroy_stream_data_vec(&graph->stream_data, alc);
}

stream_t create_stream(graph_t *graph, allocator_t *alc, errors_t *errors) {
  return create_streams(&graph->stream_data, 1, alc, errors);
}

tap_t tap_stream(graph_t *graph, stream_t stream, allocator_t *alc, errors_t *errors) {
  stream_data_t *data = &graph->stream_data.ary[stream];

  if (tap_vec_push(&data->taps, graph->n_taps, alc, errors) < 0) {
    return NIL_TAP;
  }

  return graph->n_taps++;
}

void untap_stream(graph_t *graph, stream_t stream, allocator_t *alc) {
  stream_data_t *data = &graph->stream_data.ary[stream];
  ASSERT(data->taps.size > 0);
  tap_vec_pop_n(&data->taps, 1, alc);
}

stream_processor_t *create_stream_processor(graph_t *graph,
                                            tap_t *tap0,
                                            const stream_t *in,
                                            size_t n_in,
                                            stream_t *out0,
                                            size_t n_out,
                                            allocator_t *alc,
                                            errors_t *errors) {
  if (n_out > 0) {
    stream_t out = create_streams(&graph->stream_data, n_out, alc, errors);

    if (out == NIL_STREAM) {
      return NULL;
    }

    *out0 = out;
  }

  stream_processor_t *sp = stream_processor_vec_emplace(&graph->sps, alc, errors);

  if (sp == NULL) {
    uncreate_streams(&graph->stream_data, n_out, alc);
    return NULL;
  }

  if (n_in > 0) {
    *tap0 = graph->n_taps;

    for (size_t i = 0; i < n_in; i++) {
      if (tap_stream(graph, in[i], alc, errors) == NIL_STREAM) {
        uncreate_streams(&graph->stream_data, n_out, alc);
        stream_processor_vec_unemplace(&graph->sps);

        for (size_t j = 0; j < i; j++) {
          untap_stream(graph, in[i], alc);
        }

        return NULL;
      }
    }

    assert(graph->n_taps == *tap0 + n_in);
  }

  return sp;
}

static void destroy_stream_processor(stream_processor_t *sp, allocator_t *alc) {
  switch (sp->type) {
  case SP_PATH_SOURCE:
    a_free(alc, sp->u.source.u.path);
    break;

  case SP_STR_SOURCE:
  case SP_BUFFER_SOURCE:
    a_free(alc, sp->u.source.u.buf.bytes);
    break;

  case SP_PATH_SINK:
    a_free(alc, sp->u.sink.u.path.path);
    break;

  case SP_COMMAND:
    a_free(alc, sp->u.command.path);
    break;

  case SP_FUNCTION:
  case SP_FD_SOURCE:
  case SP_C_FILE_SOURCE:
  case SP_STATIC_STR_SOURCE:
  case SP_STATIC_BUFFER_SOURCE:
  case SP_FD_SINK:
  case SP_C_FILE_SINK:
    break;

  default:
    ASSERT(0);
  }
}

static void destroy_stream_data(stream_data_t *sd, allocator_t *alc) {
  destroy_tap_vec(&sd->taps, alc);
}

static stream_t
create_streams(stream_data_vec_t *stream_data, size_t n, allocator_t *alc, errors_t *errors) {
  stream_t stream0 = stream_data->size;
  stream_data_t *data = stream_data_vec_emplace_n(stream_data, n, alc, errors);

  if (data == NULL) {
    return NIL_STREAM;
  }

  for (size_t i = 0; i < n; i++) {
    initialize_tap_vec(&data[i].taps);
  }

  return stream0;
}

static void uncreate_streams(stream_data_vec_t *stream_data, size_t n, allocator_t *alc) {
  assert(stream_data->size >= n);
  stream_data_vec_pop_n(stream_data, n, alc);
}
