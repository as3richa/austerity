#include "stream.h"

#define METHODS_ONLY
#define NAME tap_vec
#define CONTAINED_TYPE tap_t
#include "vec.h"

void initialize_stream(stream_t *stream) {
  initialize_tap_vec(&stream->taps);
}

void destroy_stream(graph_builder_t *g, stream_t *stream) {
  destroy_tap_vec(g, &stream->taps);
}

int tap_stream(graph_builder_t *g, stream_t *in, tap_t tap, const char *call) {
  if (tap_vec_push(g, &in->taps, tap, call) < 0) {
    return -1;
  }

  return 0;
}

void untap_stream(stream_t *stream) {
  tap_vec_pop_n(NULL, &stream->taps, 1);
}
