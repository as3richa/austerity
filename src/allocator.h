#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include "common.h"

struct argv_arena;
struct env_arena;
struct func_arena;

typedef struct {
  void *(*alloc)(void *, size_t);
  void (*free)(void *, void *);
  void *user;

  struct argv_arena *argv_arena;
  struct env_arena *env_arena;
  struct func_arena *func_arena;
} allocator_t;

void initialize_allocator(allocator_t *a,
                          void *(*alloc)(void *, size_t),
                          void (*free)(void *, void *),
                          void *user);

void destroy_allocator_arenas(allocator_t *a);

void *a_alloc(const allocator_t *a, size_t size);

void *a_realloc(const allocator_t *a, void *ptr, size_t elem_size, size_t size, size_t prev);

void a_free(const allocator_t *a, void *ptr);

char *a_copy_buffer(const allocator_t *a, const char *buffer, size_t size);

char *a_copy_str(const allocator_t *a, const char *str);

argv_t *a_alloc_argv(allocator_t *a);

environment_t *a_alloc_env(allocator_t *a);

func_t *a_alloc_func(allocator_t *a);

#endif
