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

// FIXME: lay this all out better

typedef struct austerity_graph_builder austerity_graph_builder_t;

austerity_graph_builder_t *austerity_create_graph_builder(void);

austerity_graph_builder_t *austerity_create_graph_builder_a(void *(*alloc)(void *, size_t),
                                                            void (*free)(void *, void *),
                                                            void *user);

void austerity_destroy_graph_builder(austerity_graph_builder_t *g);

typedef struct austerity_error austerity_error_t;

void austerity_graph_builder_abort_on_error(austerity_graph_builder_t *g);

void austerity_graph_builder_on_error(austerity_graph_builder_t *g,
                                      void (*callback)(void *, const austerity_error_t *),
                                      void *user);

typedef uint_least32_t austerity_stream_t;
#define AUSTERITY_NIL_STREAM (austerity_stream_t)(-1)
#define AUSTERITY_STREAM_VA_END (austerity_stream_t)(-2)
#define AUSTERITY_MAX_STREAMS (austerity_stream_t)(-3)

austerity_stream_t austerity_fd_source(austerity_graph_builder_t *g, int fd);

austerity_stream_t austerity_path_source(austerity_graph_builder_t *g, const char *path);

austerity_stream_t austerity_c_file_source(austerity_graph_builder_t *g, FILE *c_file);

austerity_stream_t austerity_str_source(austerity_graph_builder_t *g, const char *str);

austerity_stream_t austerity_static_str_source(austerity_graph_builder_t *g, const char *str);

austerity_stream_t
austerity_buffer_source(austerity_graph_builder_t *g, const char *data, size_t size);

austerity_stream_t
austerity_static_buffer_source(austerity_graph_builder_t *g, const char *data, size_t size);

int austerity_fd_sink(austerity_graph_builder_t *g, int fd, austerity_stream_t in);

int austerity_path_sink(austerity_graph_builder_t *g,
                        const char *path,
                        int append,
                        austerity_stream_t in);

int austerity_c_file_sink(austerity_graph_builder_t *g, FILE *c_file, austerity_stream_t in);

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

austerity_argv_t *austerity_create_argv_v(austerity_graph_builder_t *g,
                                          ... /* const char *, ..., NULL */);

int austerity_argv_push_str(austerity_argv_t *argv, const char *arg);

int austerity_argv_push_strs(austerity_argv_t *argv, char **const arg, size_t n_args);

int austerity_argv_push_strs_v(austerity_argv_t *argv, ... /* const char * */);

int austerity_argv_push_input(austerity_argv_t *argv, austerity_stream_t in);

austerity_stream_t austerity_argv_push_output(austerity_argv_t *argv);

austerity_stream_t austerity_command(austerity_graph_builder_t *g,
                                     const char *path,
                                     austerity_argv_t *argv,
                                     austerity_stream_t stdin);

austerity_stream_t austerity_command_e(austerity_graph_builder_t *g,
                                       austerity_stream_t *stderr,
                                       austerity_environment_t *env,
                                       const char *path,
                                       austerity_argv_t *argv,
                                       austerity_stream_t stdin);

// ============================================================================
// Functions
// ============================================================================

typedef struct austerity_func austerity_func_t;

austerity_func_t *
austerity_create_func(austerity_graph_builder_t *g,
                      const char *name,
                      int (*callback)(void *, const int *, size_t, const int *, size_t),
                      size_t n_out);

size_t austerity_func_n_out(const austerity_func_t *func);

int austerity_function(austerity_graph_builder_t *g,
                       austerity_stream_t *out,
                       austerity_func_t *func,
                       void *user,
                       const austerity_stream_t *in,
                       size_t n_in);

int austerity_function_0(austerity_graph_builder_t *g,
                         austerity_stream_t *out,
                         austerity_func_t *func,
                         void *user);

int austerity_function_1(austerity_graph_builder_t *g,
                         austerity_stream_t *out,
                         austerity_func_t *func,
                         void *user,
                         austerity_stream_t in);

int austerity_function_2(austerity_graph_builder_t *g,
                         austerity_stream_t *out,
                         austerity_func_t *func,
                         void *user,
                         austerity_stream_t left,
                         austerity_stream_t right);

int austerity_function_e(austerity_graph_builder_t *g,
                         austerity_stream_t *out,
                         austerity_environment_t *env,
                         austerity_func_t *func,
                         void *user,
                         const austerity_stream_t *in,
                         size_t n_in);

int austerity_function_v(austerity_graph_builder_t *g,
                         austerity_stream_t *out,
                         austerity_func_t *func,
                         void *user,
                         ... /* austerity_stream_t, ..., AUSTERITY_STREAM_VA_END */);

typedef struct austerity_program austerity_program_t;

austerity_program_t *austerity_compile_graph(austerity_graph_builder_t *g);

#ifdef AUSTERITY_ABBREV
#include "austerity-abbrev.h"
#endif

#ifdef __cplusplus
}
#endif

#endif
