#include "alloc.h"
#include "common.h"

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

static __attribute__((unused)) void L(initialize)(TYPE **arena) {
  *arena = NULL;
}

static __attribute__((unused)) void L(destroy)(graph_builder_t *g, TYPE **arena) {
  for (TYPE *it = *arena, *next; it != NULL; it = next) {
#ifdef DESTRUCTOR
    for (size_t i = 0; i < it->size; i++) {
      DESTRUCTOR(g, &it->ary[i]);
    }
#endif

    next = it->next;
    ifree(g, it);
  }
}

static __attribute__((unused)) CONTAINED_TYPE *
M(alloc)(graph_builder_t *g, TYPE **arena_ref, const char *call) {
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
    arena = ialloc(g, size, call);

    if (arena == NULL) {
      return NULL;
    }

    *arena = (TYPE){0, capacity, next};
    *arena_ref = arena;
  }

  ASSERT(arena != NULL && arena->size < arena->capacity);

  CONTAINED_TYPE *elem = &arena->ary[arena->size++];

#ifdef CONSTRUCTOR
  CONSTRUCTOR(g, elem);
#endif

  return elem;
}

#undef HEADER

#undef NAME
#undef CONTAINED_TYPE
#undef CONSTRUCTOR
#undef DESTRUCTOR

#undef PASTE2
#undef PASTE
#undef L
#undef M
#undef TYPE
