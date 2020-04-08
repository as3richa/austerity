#include "common.h"
#include "dsl.h"

stream_t *command(graph_builder_t *g, const char *path, argv_t *argv, stream_t *stdin);

stream_t *command_e(stream_t **stderr,
                    graph_builder_t *g,
                    const char *path,
                    argv_t *argv,
                    environment_t *env,
                    stream_t *stdin);
