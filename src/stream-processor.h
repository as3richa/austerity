#ifndef STREAM_PROCESSOR_H
#define STREAM_PROCESSOR_H

typedef st_size_t tap_t;

struct buffer {
  char *bytes;
  st_size_t size;
};

struct static_buffer {
  const char *bytes;
  st_size_t size;
};

union sp_source {
  int fd;
  char *path;
  FILE *c_file;
  struct buffer buf;
  struct static_buffer sbuf;
};

struct sp_sink {
  tap_t in;

  union {
    int fd;

    struct wpath {
      char *path;
      unsigned int append : 1;
    } path;

    FILE *c_file;
  } u;
};

struct sp_command {
  environment_t *env;
  char *path;
  argv_t *argv;
  tap_t stdin;
};

struct sp_function {
  austerity_func_t *func;
  tap_t *in;
  size_t n_in;
};

typedef struct {
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

  stream_t *out;

  union {
    union sp_source source;
    struct sp_sink sink;
    struct sp_command command;
    struct sp_function function;
  } u;
} stream_processor_t;

#endif
