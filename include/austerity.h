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

austerity_graph_builder_t *austerity_create_graph_builder(void);

austerity_graph_builder_t *austerity_create_graph_builder_a(void *user,
                                                            void *(*alloc)(size_t, void *),
                                                            void (*free)(void *, void *));

austerity_graph_builder_t *austerity_create_graph_builder_p(void);

void austerity_destroy_graph_builder(austerity_graph_builder_t *g);

// ============================================================================
// Sources
// ============================================================================

typedef uint_least32_t austerity_source_t;
#define AUSTERITY_DEV_NULL (austerity_source_t)0xfffffffful

austerity_source_t austerity_fd_source(austerity_graph_builder_t *g, int fd);

austerity_source_t austerity_file_source(austerity_graph_builder_t *g, const char *path, int flags);

austerity_source_t austerity_stdio_source(austerity_graph_builder_t *g, FILE *file);

austerity_source_t austerity_str_source(austerity_graph_builder_t *g, const char *str);

austerity_source_t
austerity_buffer_source(austerity_graph_builder_t *g, size_t size, const char *data);

austerity_source_t
austerity_static_buffer_source(austerity_graph_builder_t *g, size_t size, const char *data);

austerity_source_t austerity_io_param_source(austerity_graph_builder_t *g, size_t index);

// ============================================================================
// Sinks
// ============================================================================

int austerity_fd_sink(austerity_graph_builder_t *g, int fd, austerity_source_t source);

int austerity_file_sink(austerity_graph_builder_t *g,
                        const char *path,
                        int flags,
                        austerity_source_t source);

int austerity_stdio_sink(austerity_graph_builder_t *g, FILE *file, austerity_source_t source);

int austerity_io_param_sink(austerity_graph_builder_t *g, size_t index, austerity_source_t source);

// ============================================================================
// Environments
// ============================================================================

typedef struct austerity_environment austerity_environment_t;

austerity_environment_t *austerity_create_environment(austerity_graph_builder_t *g);

void austerity_environment_setwd(austerity_environment_t *env, const char *path);

void austerity_environment_keepenv(austerity_environment_t *env);

void austerity_environment_clearenv(austerity_environment_t *env);

int austerity_environment_setenv(austerity_environment_t *env,
                                 const char *name,
                                 const char *value,
                                 int overwrite);

int austerity_environment_unsetenv(austerity_environment_t *env, const char *name);

void austerity_environment_setreuid(austerity_environment_t *env, uid_t ruid, uid_t euid);

void austerity_environment_setregid(austerity_environment_t *env, gid_t rgid, gid_t egid);

void austerity_set_default_environment(austerity_graph_builder_t *g, austerity_environment_t *env);

// ============================================================================
// Commands
// ============================================================================

typedef struct austerity_command_builder austerity_command_builder_t;

austerity_command_builder_t *austerity_create_command_builder(austerity_graph_builder_t *g,
                                                              const char *path);

int austerity_command_push_arg_str(austerity_command_builder_t *cmd, const char *arg);

int austerity_command_push_arg_source(austerity_command_builder_t *cmd, austerity_source_t arg);

int austerity_command_push_arg_str_param(austerity_command_builder_t *cmd,
                                         size_t index,
                                         const char *dfault);

int austerity_command_push_arg_io_param(austerity_command_builder_t *cmd, size_t index);

void austerity_command_set_environment(austerity_command_builder_t *cmd,
                                       austerity_environment_t *env);

void austerity_command_set_argv0(austerity_command_builder_t *cmd);

typedef struct austerity_command_result {
  austerity_source_t stdout;
  austerity_source_t stderr;
} austerity_command_result_t;

austerity_command_result_t austerity_command(austerity_graph_builder_t *g,
                                             austerity_command_builder_t *cmd,
                                             austerity_source_t stdin);

// ============================================================================
// Functions
// ============================================================================

typedef struct austerity_function_builder austerity_function_builder_t;

typedef struct austerity_function_io {
  int fd;
  const char *path;
} austerity_fn_io_t;

typedef int (*austerity_function_t)(
    size_t n_in, austerity_fn_io_t *in, size_t n_out, austerity_fn_io_t *out, void *user);

austerity_function_builder_t *austerity_create_function_builder(austerity_graph_builder_t *g,
                                                                size_t n_in,
                                                                size_t n_out,
                                                                austerity_function_t func);

void austerity_function_set_user_data(austerity_function_builder_t *fn, void *user);

void austerity_function_set_environment(austerity_function_builder_t *fn,
                                        austerity_environment_t *env);

int austerity_function_needs_path_in(austerity_function_builder_t *fn, size_t index);

int austerity_function_needs_path_in_m(austerity_function_builder_t *fn, size_t mask);

int austerity_function_needs_path_out(austerity_function_builder_t *fn, size_t index);

int austerity_function_needs_path_out_m(austerity_function_builder_t *fn, size_t mask);

austerity_source_t *
austerity_function(austerity_graph_builder_t *g, austerity_function_builder_t *fn, ...);

#ifdef AUSTERITY_ABBREV

// FIXME

#endif

#ifdef __cplusplus
}
#endif

#endif
