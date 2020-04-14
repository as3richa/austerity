#include "argv.h"
#include "common.h"
#include "graph-builder.h"
#include "graph.h"
#include "stream-processor.h"

static stream_t my_command_e(graph_builder_t *g,
                             stream_t *stderr,
                             const char *path,
                             argv_t *argv,
                             environment_t *env,
                             stream_t stdin,
                             const char *call);

stream_t command(graph_builder_t *g, const char *path, argv_t *argv, stream_t stdin) {
  return my_command_e(g, NULL, path, argv, NULL, stdin, __func__);
}

stream_t command_e(graph_builder_t *g,
                   stream_t *stderr,
                   environment_t *env,
                   const char *path,
                   argv_t *argv,
                   stream_t stdin) {
  return my_command_e(g, stderr, path, argv, env, stdin, __func__);
}

static stream_t my_command_e(graph_builder_t *g,
                             stream_t *stderr,
                             const char *path,
                             argv_t *argv,
                             environment_t *env,
                             stream_t stdin,
                             const char *call) {
  if (env == NULL) {
    env = g->default_env;
  }

  char *my_path = g_copy_str(g, path, call);

  if (my_path == NULL) {
    if (stderr != NULL) {
      *stderr = NIL_STREAM;
    }

    return NIL_STREAM;
  }

  tap_t tap;
  stream_t stdout_stderr;
  stream_processor_t *sp =
      create_stream_processor(g, &g->gr, &tap, &stdin, 1, &stdout_stderr, 2, call);

  if (sp == NULL) {
    g_free(g, my_path);

    if (stderr != NULL) {
      *stderr = NIL_STREAM;
    }

    return NIL_STREAM;
  }

  sp->type = SP_COMMAND;
  sp->u.command = (struct sp_command){env, my_path, argv, tap, stdout_stderr};

  // FIXME: handle default value of argv, somewhere
  if (argv != NULL) {
    argv->used = 1;
  }

  if (stderr != NULL) {
    *stderr = stdout_stderr + 1;
  }

  return stdout_stderr;
}
