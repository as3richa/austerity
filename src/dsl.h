#ifndef DSL_H
#define DSL_H

#include "common.h"
#include "stream-processor.h"

#define TYPE_ONLY
#define NAME tap_vec
#define CONTAINED_TYPE tap_t
#include "vec.h"

struct austerity_stream {
  tap_vec_t taps;
};

#define TYPE_ONLY
#define NAME sp_vec
#define CONTAINED_TYPE stream_processor_t
#include "vec.h"

typedef struct {
  sp_vec_t sps;
  tap_t n_taps;
} dsl_state_t;

void initialize_dsl_state(dsl_state_t *dsl);
void destroy_dsl_state(graph_builder_t *g, dsl_state_t *dsl);

stream_processor_t *emplace_stream_processor(graph_builder_t *g,
                                             tap_t *tap0,
                                             stream_t *const *in,
                                             size_t n_in,
                                             size_t n_out,
                                             const char *call);

#endif
