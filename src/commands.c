#include "common.h"
#include "dsl.h"
#include "graph-builder.h"

static stream_t *my_command_e(stream_t **stderr,
                              graph_builder_t *g,
                              const char *path,
                              argv_t *argv,
                              environment_t *env,
                              stream_t *stdin,
                              const char *api_fn_name);

stream_t *command(graph_builder_t *g, const char *path, argv_t *argv, stream_t *stdin) {
  return my_command_e(NULL, g, path, argv, graph_builder_default_env(g), stdin, __func__);
}

stream_t *command_e(stream_t **stderr,
                    graph_builder_t *g,
                    const char *path,
                    argv_t *argv,
                    environment_t *env,
                    stream_t *stdin) {
  return my_command_e(stderr, g, path, argv, env, stdin, __func__);
}

static stream_t *my_command_e(stream_t **stderr,
                              graph_builder_t *g,
                              const char *path,
                              argv_t *argv,
                              environment_t *env,
                              stream_t *stdin,
                              const char *api_fn_name) {
  char *my_path = copy_str(g, path, api_fn_name);

  if (my_path == NULL) {
    if (stderr != NULL) {
      *stderr = NULL;
    }

    return NULL;
  }

  tap_t tap;
  stream_processor_t *sp = emplace_stream_processor(g, &tap, &stdin, 1, 2, api_fn_name);

  if (sp == NULL) {
    ifree(g, my_path);

    if (stderr != NULL) {
      *stderr = NULL;
    }

    return NULL;
  }

  sp->type = SP_COMMAND;
  sp->u.command = (struct sp_command){env, my_path, argv, tap};

  if (stderr != NULL) {
    *stderr = &sp->out[1];
  }

  return &sp->out[0];
}
