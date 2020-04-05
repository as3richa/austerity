#include "common.h"
#include "stream-processor.h"

struct austerity_argv {
  int fixme;
};

argv_t *create_argv(graph_builder_t *g);

argv_t *create_argv_v(graph_builder_t *g);

int argv_push_str(argv_t *argv, const char *str);

int argv_push_strs(argv_t *argv, char **const strs, size_t n_strs);

int argv_push_strs_v(argv_t *argv, ...);

int argv_push_stream(argv_t *argv, stream_t *in);

stream_t *command(graph_builder_t *g, const char *path, argv_t *argv, stream_t *stdin);

stream_t *command_e(stream_t **stderr,
                    graph_builder_t *g,
                    const char *path,
                    argv_t *argv,
                    environment_t *env,
                    stream_t *stdin);
