#ifndef STREAM_H
#define STREAM_H

#include "common.h"

typedef st_size_t tap_t;
#define DEV_NULL (tap_t) - 1

#define TYPE_ONLY
#define NAME tap_vec
#define CONTAINED_TYPE tap_t
#include "vec.h"

struct austerity_stream {
  tap_vec_t taps;
};

void initialize_stream(stream_t *stream);
void destroy_stream(graph_builder_t *g, stream_t *stream);

int tap_stream(graph_builder_t *g, stream_t *in, tap_t tap, const char *call);
void untap_stream(stream_t *stream);

#endif
