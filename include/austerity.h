#ifndef AUSTERITY_H
#define AUSTERITY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

typedef struct austerity_graph_builder austerity_graph_builder_t;

typedef struct austerity_error {
  const char *api_fn_name;
  int errnum;
  const char *english;
} austerity_error_t;

austerity_graph_builder_t *austerity_create_graph_builder(void);

austerity_graph_builder_t *austerity_create_graph_builder_a(void *(*alloc)(size_t, void *),
                                                            void (*free)(void *, void *),
                                                            void *user);

void austerity_graph_builder_abort_on_error(austerity_graph_builder_t *g);

void austerity_graph_builder_abort_on_error_c(austerity_graph_builder_t *g,
                                              void (*callback)(austerity_error_t *, void *),
                                              void *user);

void austerity_destroy_graph_builder(austerity_graph_builder_t *g);

// ============================================================================
// Sources
// ============================================================================

typedef struct austerity_stream austerity_stream_t;

austerity_stream_t *austerity_fd_source(austerity_graph_builder_t *g, int fd);

austerity_stream_t *austerity_path_source(austerity_graph_builder_t *g, const char *path);

austerity_stream_t *austerity_c_file_source(austerity_graph_builder_t *g, FILE *c_file);

austerity_stream_t *austerity_str_source(austerity_graph_builder_t *g, const char *str);

austerity_stream_t *austerity_static_str_source(austerity_graph_builder_t *g, const char *str);

austerity_stream_t *
austerity_buffer_source(austerity_graph_builder_t *g, const char *data, size_t size);

austerity_stream_t *
austerity_static_buffer_source(austerity_graph_builder_t *g, const char *data, size_t size);

// ============================================================================
// Sinks
// ============================================================================

int austerity_fd_sink(austerity_graph_builder_t *g, int fd, austerity_stream_t *in);

int austerity_path_sink(austerity_graph_builder_t *g,
                        const char *path,
                        int append,
                        austerity_stream_t *in);

int austerity_c_file_sink(austerity_graph_builder_t *g, FILE *c_file, austerity_stream_t *in);

// ============================================================================
// Environments
// ============================================================================

typedef struct austerity_environment austerity_environment_t;

austerity_environment_t *austerity_create_environment(austerity_graph_builder_t *g);

int austerity_environment_setwd(austerity_environment_t *env, const char *path);

int austerity_environment_preserve_env(austerity_environment_t *env);

int austerity_environment_clearenv(austerity_environment_t *env);

int austerity_environment_setenv(austerity_environment_t *env,
                                 const char *name,
                                 const char *value,
                                 int overwrite);

int austerity_environment_unsetenv(austerity_environment_t *env, const char *name);

int austerity_environment_setreuid(austerity_environment_t *env, uid_t ruid, uid_t euid);

int austerity_environment_setregid(austerity_environment_t *env, gid_t rgid, gid_t egid);

int austerity_set_default_environment(austerity_graph_builder_t *g, austerity_environment_t *env);

// ============================================================================
// Commands
// ============================================================================

typedef struct austerity_argv austerity_argv_t;

austerity_argv_t *austerity_create_argv(austerity_graph_builder_t *g);

austerity_argv_t *austerity_create_argv_v(austerity_graph_builder_t *g);

int austerity_argv_push_str(austerity_argv_t *argv, const char *str);

int austerity_argv_push_strs(austerity_argv_t *argv, char **const strs, size_t n_strs);

int austerity_argv_push_strs_v(austerity_argv_t *argv, ...);

int austerity_argv_push_source(austerity_argv_t *argv, austerity_stream_t *in);

austerity_stream_t *austerity_command(austerity_graph_builder_t *g,
                                      const char *path,
                                      austerity_argv_t *argv,
                                      austerity_stream_t *stdin);

austerity_stream_t *austerity_command_e(austerity_stream_t **stderr,
                                        austerity_graph_builder_t *g,
                                        const char *path,
                                        austerity_argv_t *argv,
                                        austerity_environment_t *env,
                                        austerity_stream_t *stdin);

// ============================================================================
// Functions
// ============================================================================

typedef int (*austerity_function_t)(int *in, size_t n_in, int *out, size_t n_out, void *user);

int austerity_function(austerity_stream_t **out,
                       size_t n_out,
                       austerity_graph_builder_t *g,
                       austerity_function_t fn,
                       const char *name,
                       void *user,
                       const austerity_stream_t **in,
                       size_t n_in);

int austerity_function_0(austerity_stream_t **out,
                         size_t n_out,
                         austerity_graph_builder_t *g,
                         austerity_function_t fn,
                         const char *name,
                         void *user);

int austerity_function_1(austerity_stream_t **out,
                         size_t n_out,
                         austerity_graph_builder_t *g,
                         austerity_function_t fn,
                         const char *name,
                         void *user,
                         austerity_stream_t *in);

int austerity_function_2(austerity_stream_t **out,
                         size_t n_out,
                         austerity_graph_builder_t *g,
                         austerity_function_t fn,
                         const char *name,
                         void *user,
                         austerity_stream_t *left,
                         austerity_stream_t *right);

int austerity_function_e(austerity_stream_t **out,
                         size_t n_out,
                         austerity_graph_builder_t *g,
                         austerity_function_t fn,
                         const char *name,
                         void *user,
                         austerity_environment_t *env,
                         const austerity_stream_t **in,
                         size_t n_in);

int austerity_function_v(austerity_stream_t **out,
                         size_t n_out,
                         austerity_graph_builder_t *g,
                         austerity_function_t fn,
                         const char *name,
                         void *user,
                         size_t n_in,
                         ... /* austerity_stream_t * */);

#ifdef AUSTERITY_ABBREV
#include "austerity-abbrev.h"
#endif

#ifdef __cplusplus
}
#endif

#endif
