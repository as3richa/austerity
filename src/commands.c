#include "argv.h"
#include "common.h"
#include "graph-builder.h"
#include "graph.h"
#include "stream-processor.h"

static stream_t my_command_e(graph_t *graph,
                             stream_t *stderr,
                             const char *path,
                             argv_t *argv,
                             environment_t *env,
                             stream_t stdin,
                             allocator_t *alc,
                             errors_t *errors);

stream_t command(graph_builder_t *g, const char *path, argv_t *argv, stream_t stdin) {
  return my_command_e(&g->graph, NULL, path, argv, NULL, stdin, &g->alc, &g->errors);
}

stream_t command_e(graph_builder_t *g,
                   stream_t *stderr,
                   environment_t *env,
                   const char *path,
                   argv_t *argv,
                   stream_t stdin) {
  if (env == NULL) {
    env = g->default_env;
  }

  return my_command_e(&g->graph, stderr, path, argv, env, stdin, &g->alc, &g->errors);
}

static stream_t my_command_e(graph_t *graph,
                             stream_t *stderr,
                             const char *path,
                             argv_t *argv,
                             environment_t *env,
                             stream_t stdin,
                             allocator_t *alc,
                             errors_t *errors) {
  char *my_path = a_copy_str(alc, path, errors);

  if (my_path == NULL) {
    if (stderr != NULL) {
      *stderr = NIL_STREAM;
    }

    return NIL_STREAM;
  }

  tap_t tap;
  stream_t stdout_stderr;
  stream_processor_t *sp =
      create_stream_processor(graph, &tap, &stdin, 1, &stdout_stderr, 2, alc, errors);

  if (sp == NULL) {
    a_free(alc, my_path);

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
