#include "graph.h"
#include "graph-builder.h"
#include "stream-processor.h"

static void destroy_stream_processor(graph_builder_t *g, stream_processor_t *sp);

#define METHODS_ONLY
#define NAME stream_processor_vec
#define CONTAINED_TYPE stream_processor_t
#define DESTRUCTOR destroy_stream_processor
#include "vec.h"

#define METHODS_ONLY
#define NAME tap_vec
#define CONTAINED_TYPE tap_t
#include "vec.h"

static void destroy_stream_data(graph_builder_t *g, stream_data_t *sd);

#define METHODS_ONLY
#define NAME stream_data_vec
#define CONTAINED_TYPE stream_data_t
#define DESTRUCTOR destroy_stream_data
#include "vec.h"

static stream_t
create_streams(graph_builder_t *g, stream_data_vec_t *stream_data, size_t n, const char *call);

static void uncreate_streams(graph_builder_t *g, stream_data_vec_t *stream_data, size_t n);

void initialize_graph(struct graph *gr) {
  initialize_stream_processor_vec(&gr->sps);
  initialize_stream_data_vec(&gr->stream_data);
  gr->n_taps = 0;
}

void destroy_graph(graph_builder_t *g, struct graph *gr) {
  destroy_stream_processor_vec(g, &gr->sps);
  destroy_stream_data_vec(g, &gr->stream_data);
}

int shallow_copy_graph_from_builder(graph_builder_t *g, struct graph *dest, const char *call) {
  struct graph *gr = &g->gr;

  if (shallow_copy_stream_processor_vec(g, &dest->sps, &gr->sps, call) < 0) {
    return -1;
  }

  if (shallow_copy_stream_data_vec(g, &dest->stream_data, &gr->stream_data, call)) {
    shallow_destroy_stream_processor_vec(g, &dest->sps);
    return -1;
  }

  dest->n_taps = gr->n_taps;
  return 0;
}

void shallow_destroy_graph(graph_builder_t *g, struct graph *gr) {
  shallow_destroy_stream_processor_vec(g, &gr->sps);
  shallow_destroy_stream_data_vec(g, &gr->stream_data);
}

stream_t create_stream(graph_builder_t *g, struct graph *gr, const char *call) {
  return create_streams(g, &gr->stream_data, 1, call);
}

tap_t tap_stream(graph_builder_t *g, struct graph *gr, stream_t stream, const char *call) {
  stream_data_t *data = stream_data_vec_ref(&gr->stream_data, stream);

  if (tap_vec_push(g, &data->taps, gr->n_taps, call) < 0) {
    return NIL_TAP;
  }

  return gr->n_taps++;
}

void untap_stream(graph_builder_t *g, struct graph *gr, stream_t stream) {
  stream_data_t *data = stream_data_vec_ref(&gr->stream_data, stream);
  assert(data->taps.size > 0);
  tap_vec_pop_n(g, &data->taps, 1); // FIXME: _pop?
}

stream_processor_t *create_stream_processor(graph_builder_t *g,
                                            struct graph *gr,
                                            tap_t *tap0,
                                            const stream_t *in,
                                            size_t n_in,
                                            stream_t *out0,
                                            size_t n_out,
                                            const char *call) {
  if (n_out > 0) {
    stream_t out = create_streams(g, &gr->stream_data, n_out, call);

    if (out == NIL_STREAM) {
      return NULL;
    }

    *out0 = out;
  }

  stream_processor_t *sp = stream_processor_vec_emplace(g, &gr->sps, call);

  if (sp == NULL) {
    uncreate_streams(g, &gr->stream_data, n_out);
    return NULL;
  }

  if (n_in > 0) {
    *tap0 = gr->n_taps;

    for (size_t i = 0; i < n_in; i++) {
      if (tap_stream(g, gr, in[i], call) == NIL_STREAM) {
        uncreate_streams(g, &gr->stream_data, n_out);
        stream_processor_vec_unemplace(&gr->sps);

        for (size_t j = 0; j < i; j++) {
          untap_stream(g, gr, in[i]);
        }

        return NULL;
      }
    }

    assert(gr->n_taps == *tap0 + n_in);
  }

  return sp;
}

static void destroy_stream_processor(graph_builder_t *g, stream_processor_t *sp) {
  switch (sp->type) {
  case SP_PATH_SOURCE:
    ifree(g, sp->u.source.u.path);
    break;

  case SP_STR_SOURCE:
  case SP_BUFFER_SOURCE:
    ifree(g, sp->u.source.u.buf.bytes);
    break;

  case SP_PATH_SINK:
    ifree(g, sp->u.sink.u.path.path);
    break;

  case SP_COMMAND:
    ifree(g, sp->u.command.path);
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

static void destroy_stream_data(graph_builder_t *g, stream_data_t *sd) {
  destroy_tap_vec(g, &sd->taps);
}

static stream_t
create_streams(graph_builder_t *g, stream_data_vec_t *stream_data, size_t n, const char *call) {
  stream_t stream0 = stream_data->size;
  stream_data_t *data = stream_data_vec_emplace_n(g, stream_data, n, call);

  if (data == NULL) {
    return NIL_STREAM;
  }

  for (size_t i = 0; i < n; i++) {
    initialize_tap_vec(&data[i].taps);
  }

  return stream0;
}

static void uncreate_streams(graph_builder_t *g, stream_data_vec_t *stream_data, size_t n) {
  assert(stream_data->size >= n);
  stream_data_vec_pop_n(g, stream_data, n);
}
