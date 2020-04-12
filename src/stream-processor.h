#ifndef STREAM_PROCESSOR_H
#define STREAM_PROCESSOR_H

struct buffer {
  char *bytes;
  st_size_t size;
};

struct static_buffer {
  const char *bytes;
  st_size_t size;
};

struct sp_source {
  stream_t out;

  union {
    int fd;
    char *path;
    FILE *c_file;
    struct buffer buf;
    struct static_buffer sbuf;
  } u;
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
  stream_t stdout_stderr;
};

struct sp_function {
  environment_t *env;
  austerity_func_t *func;
  void *user;
  tap_t in0;
  st_size_t n_in;
  stream_t out0;
  st_size_t n_out;
};

struct sp_splitter {
  tap_t in;
  stream_t out0;
  st_size_t n_out;
};

struct sp_joiner {
  tap_t *in;
  st_size_t n_in;
  stream_t out;
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
    SP_COMMAND,
    SP_FUNCTION,
    SP_SPLITTER,
    SP_JOINER
  } type;

  union {
    struct sp_source source;
    struct sp_sink sink;
    struct sp_command command;
    struct sp_function function;
    struct sp_splitter splitter;
    struct sp_joiner joiner;
  } u;
} stream_processor_t;

#endif
