#include "allocator.h"
#include "argv.h"
#include "environment.h"
#include "errors.h"
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

void initialize_allocator(allocator_t *alc,
                          void *(*alloc)(void *, size_t),
                          void (*free)(void *, void *),
                          void *user) {
  alc->alloc = alloc;
  alc->free = free;
  alc->user = user;
  initialize_argv_arena(&alc->argv_arena);
  initialize_env_arena(&alc->env_arena);
  initialize_func_arena(&alc->func_arena);
}

void destroy_allocator_arenas(allocator_t *alc) {
  destroy_argv_arena(&alc->argv_arena, alc);
  destroy_env_arena(&alc->env_arena, alc);
  destroy_func_arena(&alc->func_arena, alc);
}

#define CHECK_AND_RETURN_PTR(ptr, errors)                                                          \
  do {                                                                                             \
    if ((ptr) == NULL) {                                                                           \
      record_alloc_failure(errors);                                                                \
      return NULL;                                                                                 \
    }                                                                                              \
    return (ptr);                                                                                  \
  } while (0)

void *a_alloc(const allocator_t *alc, size_t size, errors_t *errors) {
  void *ptr = (*alc->alloc)(alc->user, size);
  CHECK_AND_RETURN_PTR(ptr, errors);
}

void *a_realloc(const allocator_t *alc,
                void *ptr,
                size_t elem_size,
                size_t size,
                size_t prev,
                errors_t *errors) {
  ASSERT(size > prev);

  void *next = a_alloc(alc, elem_size * size, errors);

  if (next == NULL) {
    record_alloc_failure(errors);
    return NULL;
  }

  memcpy(next, ptr, elem_size * prev);
  a_free(alc, ptr);

  return next;
}

void a_free(const allocator_t *alc, void *ptr) {
  (*alc->free)(alc->user, ptr);
}

char *a_copy_buffer(const allocator_t *alc, const char *buffer, size_t size, errors_t *errors) {
  char *copy = a_alloc(alc, size, errors);

  if (copy == NULL) {
    record_alloc_failure(errors);
    return NULL;
  }

  memcpy(copy, buffer, size);
  return copy;
}

char *a_copy_str(const allocator_t *alc, const char *str, errors_t *errors) {
  return a_copy_buffer(alc, str, strlen(str) + 1, errors);
}

argv_t *a_alloc_argv(allocator_t *alc, errors_t *errors) {
  argv_t *argv = argv_arena_alloc(alc, &alc->argv_arena, errors);
  CHECK_AND_RETURN_PTR(argv, errors);
}

environment_t *a_alloc_env(allocator_t *alc, errors_t *errors) {
  environment_t *env = env_arena_alloc(alc, &alc->env_arena, errors);
  CHECK_AND_RETURN_PTR(env, errors);
}

func_t *a_alloc_func(allocator_t *alc, errors_t *errors) {
  func_t *func = func_arena_alloc(alc, &alc->func_arena, errors);
  CHECK_AND_RETURN_PTR(func, errors);
}
