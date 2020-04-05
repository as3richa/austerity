#ifndef STREAM_PROCESSOR_H
#define STREAM_PROCESSOR_H

#include "common.h"
#include "graph-builder.h"

struct austerity_source {
  struct tap_vec {
    tap_t *ary;
    st_size_t size;
    st_size_t capacity;
  } taps;
};

struct stream_processor {
  enum {
    SP_FD_SOURCE,
    SP_PATH_SOURCE,
    SP_C_FILE_SOURCE,
    SP_STR_SOURCE,
    SP_BUFFER_SOURCE,
    SP_STATIC_STR_SOURCE,
    SP_STATIC_BUFFER_SOURCE,
    SP_FD_SINK,
    SP_PATH_SINK,
    SP_C_FILE_SINK,
    SP_COMMAND
  } type;

  st_size_t n_out;
  source_t *out;

  union {
    int fd;

    struct {
      const char *path;
      unsigned int append : 1;
    } path;

    FILE *c_file;

    struct buffer {
      char *bytes;
      st_size_t size;
    } buffer;

    struct static_buffer {
      const char *bytes;
      st_size_t size;
    } static_buffer;

    struct {
      environment_t *env;
      char *path;
      argv_t *argv;
      tap_t stdin;
    } command;
  } data;
};

struct stream_processor *emplace_stream_processor(tap_t *tap0,
                                                  graph_builder_t *g,
                                                  source_t **in,
                                                  size_t n_in,
                                                  size_t n_out,
                                                  const char *api_fn_name);

#endif
