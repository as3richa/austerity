#ifndef ARENA_H
#define ARENA_H

#include "common.h"
#include "graph-builder.h"

#define PASTE2(x, y) x##y
#define PASTE(x, y) PASTE2(x, y)

#define ARENA_STRUCT(contained_type) PASTE(contained_type, _arena)

#define ARENA_STRUCT_DECL(contained_type)                                                          \
  struct ARENA_STRUCT(contained_type) {                                                            \
    st_size_t size;                                                                                \
    st_size_t capacity;                                                                            \
    struct ARENA_STRUCT(contained_type) * next;                                                    \
    contained_type ary[];                                                                          \
  }

#define ARENA_ALLOC(contained_type, result, g, arena, ret)                                         \
  do {                                                                                             \
    const size_t max_capacity =                                                                    \
        (ST_SIZE_MAX - sizeof(struct ARENA_STRUCT(contained_type))) / sizeof(contained_type);      \
    const int null = *(arena) == NULL;                                                             \
    assert(null || (*(arena))->size <= (*(arena))->capacity);                                      \
    if (null || (*(arena))->size == (*(arena))->capacity) {                                        \
      size_t capacity = null ? 1 : (2 * (*(arena))->capacity + 1);                                 \
      if (capacity > max_capacity) {                                                               \
        capacity = max_capacity;                                                                   \
      }                                                                                            \
      struct ARENA_STRUCT(contained_type) *next =                                                  \
          ialloc((g),                                                                              \
                 sizeof(struct ARENA_STRUCT(contained_type)) + sizeof(contained_type) * capacity,  \
                 __func__);                                                                        \
      if (next == NULL) {                                                                          \
        return (ret);                                                                              \
      }                                                                                            \
      next->size = 0;                                                                              \
      next->capacity = capacity;                                                                   \
      next->next = *(arena);                                                                       \
      *(arena) = next;                                                                             \
    }                                                                                              \
    assert((*(arena))->size < (*(arena))->capacity);                                               \
    *(result) = &(*(arena))->ary[(*(arena))->size++];                                              \
  } while (0)

#define DESTROY_ARENA(contained_type, g, arena, destructor)                                        \
  do {                                                                                             \
    for (struct ARENA_STRUCT(contained_type) *it = *(arena), *next; it != NULL; it = next) {       \
      for (size_t i = 0; i < it->size; i++) {                                                      \
        (*(destructor))(&it->ary[i]);                                                              \
      }                                                                                            \
      next = it->next;                                                                             \
      ifree(g, it);                                                                                \
    }                                                                                              \
  } while (0)

#endif
