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

austerity_graph_builder_t *austerity_create_graph_builder_a(void *(*alloc)(size_t, void *),
                                                            void (*free)(void *, void *),
                                                            void *user);

void austerity_graph_builder_abort_on_error(austerity_graph_builder_t *g);

void austerity_destroy_graph_builder(austerity_graph_builder_t *g);

// ============================================================================
// Sources
// ============================================================================

typedef uint_least32_t austerity_source_t;

#define AUSTERITY_DEV_NULL (austerity_source_t)0xfffffffful
#define AUSTERITY_MAX_SOURCES (AUSTERITY_DEV_NULL - 1)

austerity_source_t austerity_fd_source(austerity_graph_builder_t *g, int fd);

austerity_source_t austerity_file_source(austerity_graph_builder_t *g, const char *path, int flags);

austerity_source_t austerity_stdio_source(austerity_graph_builder_t *g, FILE *file);

austerity_source_t austerity_str_source(austerity_graph_builder_t *g, const char *str);

austerity_source_t austerity_static_str_source(austerity_graph_builder_t *g, const char *str);

austerity_source_t
austerity_buffer_source(austerity_graph_builder_t *g, const char *data, size_t size);

austerity_source_t
austerity_static_buffer_source(austerity_graph_builder_t *g, const char *data, size_t size);

// ============================================================================
// Sinks
// ============================================================================

int austerity_fd_sink(austerity_graph_builder_t *g, int fd, austerity_source_t source);

int austerity_file_sink(austerity_graph_builder_t *g,
                        const char *path,
                        int flags,
                        austerity_source_t source);

int austerity_stdio_sink(austerity_graph_builder_t *g, FILE *file, austerity_source_t source);

// ============================================================================
// Environments
// ============================================================================

typedef struct austerity_environment austerity_environment_t;

austerity_environment_t *austerity_create_environment(austerity_graph_builder_t *g);

int austerity_environment_setwd(austerity_environment_t *env, const char *path);

int austerity_environment_keepenv(austerity_environment_t *env);

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

typedef struct austerity_command_builder austerity_command_builder_t;

austerity_command_builder_t *austerity_create_command_builder(austerity_graph_builder_t *g,
                                                              const char *path);

int austerity_command_push_arg_str(austerity_command_builder_t *cmd, const char *arg);

int austerity_command_push_arg_source(austerity_command_builder_t *cmd, austerity_source_t arg);

int austerity_command_set_environment(austerity_command_builder_t *cmd,
                                      austerity_environment_t *env);

int austerity_command_set_argv0(austerity_command_builder_t *cmd, const char *argv0);

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
                                                                size_t n_out,
                                                                austerity_function_t func);

void austerity_function_set_user_data(austerity_function_builder_t *fn, void *user);

void austerity_function_set_environment(austerity_function_builder_t *fn,
                                        austerity_environment_t *env);

int austerity_function_needs_path(austerity_function_builder_t *fn,
                                  size_t in_mask,
                                  size_t out_mask);

austerity_source_t *
austerity_function(austerity_graph_builder_t *g, austerity_function_builder_t *fn, ...);

#ifdef AUSTERITY_ABBREV
#include "austerity-abbrev.h"
#endif

#ifdef __cplusplus
}
#endif

#endif
