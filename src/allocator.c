#include "allocator.h"
#include "argv.h"
#include "environment.h"
#include "func.h"

#define NAME argv_arena
#define CONTAINED_TYPE argv_t
#define DESTRUCTOR destroy_argv
#include "arena.h"

#define NAME env_arena
#define CONTAINED_TYPE environment_t
#define DESTRUCTOR destroy_environment
#include "arena.h"

#define NAME func_arena
#define CONTAINED_TYPE func_t
#define DESTRUCTOR destroy_func
#include "arena.h"

void initialize_allocator(allocator_t *a,
                          void *(*alloc)(void *, size_t),
                          void (*free)(void *, void *),
                          void *user) {
  a->alloc = alloc;
  a->free = free;
  a->user = user;
  initialize_argv_arena(&a->argv_arena);
  initialize_env_arena(&a->env_arena);
  initialize_func_arena(&a->func_arena);
}

void destroy_allocator_arenas(allocator_t *a) {
  destroy_argv_arena(a, &a->argv_arena);
  destroy_env_arena(a, &a->env_arena);
  destroy_func_arena(a, &a->func_arena);
}

void *a_alloc(const allocator_t *a, size_t size) {
  return (*a->alloc)(a->user, size);
}

void *a_realloc(const allocator_t *a, void *ptr, size_t elem_size, size_t size, size_t prev) {
  ASSERT(size > prev);

  void *next = a_alloc(a, elem_size * size);

  if (next == NULL) {
    return NULL;
  }

  memcpy(next, ptr, elem_size * prev);
  return next;
}

void a_free(const allocator_t *a, void *ptr) {
  (*a->free)(a->user, ptr);
}

char *a_copy_buffer(const allocator_t *a, const char *buffer, size_t size) {
  char *copy = a_alloc(a, size);

  if (copy == NULL) {
    return NULL;
  }

  memcpy(copy, buffer, size);
  return copy;
}

char *a_copy_str(const allocator_t *a, const char *str) {
  return a_copy_buffer(a, str, strlen(str) + 1);
}

argv_t *a_alloc_argv(allocator_t *a) {
  return argv_arena_alloc(a, &a->argv_arena);
}

environment_t *a_alloc_env(allocator_t *a) {
  return env_arena_alloc(a, &a->env_arena);
}

func_t *a_alloc_func(allocator_t *a) {
  return func_arena_alloc(a, &a->func_arena);
}
