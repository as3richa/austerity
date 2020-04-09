#include "common.h"
#include "dsl.h"
#include "graph-builder.h"

static stream_t *my_command_e(graph_builder_t *g,
                              stream_t **stderr,
                              const char *path,
                              argv_t *argv,
                              environment_t *env,
                              stream_t *stdin,
                              const char *call);

stream_t *command(graph_builder_t *g, const char *path, argv_t *argv, stream_t *stdin) {
  return my_command_e(g, NULL, path, argv, g->default_env, stdin, __func__);
}

stream_t *command_e(graph_builder_t *g,
                    stream_t **stderr,
                    const char *path,
                    argv_t *argv,
                    environment_t *env,
                    stream_t *stdin) {
  return my_command_e(g, stderr, path, argv, env, stdin, __func__);
}

static stream_t *my_command_e(graph_builder_t *g,
                              stream_t **stderr,
                              const char *path,
                              argv_t *argv,
                              environment_t *env,
                              stream_t *stdin,
                              const char *call) {
  char *my_path = copy_str(g, path, call);

  if (my_path == NULL) {
    if (stderr != NULL) {
      *stderr = NULL;
    }

    return NULL;
  }

  tap_t tap;
  stream_processor_t *sp = emplace_stream_processor(g, &tap, &stdin, 1, 2, call);

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
