#include "allocator.h"

#define PASTE2(x, y) x##y
#define PASTE(x, y) PASTE2(x, y)

#define L(id) PASTE(PASTE(id, _), NAME)
#define M(id) PASTE(PASTE(NAME, _), id)
#define TYPE M(t)

typedef struct NAME {
  st_size_t size;
  st_size_t capacity;
  struct NAME *next;
  CONTAINED_TYPE ary[];
} TYPE;

static void L(initialize)(TYPE **arena) {
  *arena = NULL;
}

static void L(destroy)(allocator_t *a, TYPE **arena) {
  for (TYPE *it = *arena, *next; it != NULL; it = next) {
    for (size_t i = 0; i < it->size; i++) {
      DESTRUCTOR(a, &it->ary[i]);
    }

    next = it->next;
    a_free(a, it);
  }
}

static CONTAINED_TYPE *M(alloc)(allocator_t *a, TYPE **arena_ref) {
  TYPE *arena = *arena_ref;
  ASSERT(arena == NULL || arena->size <= arena->capacity);

  if (arena == NULL || arena->size == arena->capacity) {
    size_t capacity = (arena == NULL) ? 1 : (1 + 2 * arena->capacity);

    const size_t max_capacity = (ST_SIZE_MAX - sizeof(TYPE)) / sizeof(CONTAINED_TYPE);

    if (capacity > max_capacity) {
      capacity = max_capacity;
    }

    const size_t size = sizeof(TYPE) + sizeof(CONTAINED_TYPE) * capacity;

    TYPE *next = arena;
    arena = a_alloc(a, size);

    if (arena == NULL) {
      return NULL;
    }

    *arena = (TYPE){0, capacity, next};
    *arena_ref = arena;
  }

  ASSERT(arena != NULL && arena->size < arena->capacity);
  return &arena->ary[arena->size++];
}

#undef HEADER

#undef NAME
#undef CONTAINED_TYPE
#undef DESTRUCTOR

#undef PASTE2
#undef PASTE
#undef L
#undef M
#undef TYPE
