#include "common.h"

#define PASTE2(x, y) x##y
#define PASTE(x, y) PASTE2(x, y)

#define L(id) PASTE(PASTE(id, _), NAME)
#define M(id) PASTE(PASTE(NAME, _), id)
#define TYPE M(t)

#ifndef METHODS_ONLY

typedef struct {
  CONTAINED_TYPE *ary;
  st_size_t size;
  st_size_t capacity;
} TYPE;

#endif

#ifndef TYPE_ONLY

// FIXME: gross

void *irealloc(graph_builder_t *g,
               void *ptr,
               size_t elem_size,
               size_t size,
               size_t prev,
               const char *api_fn_name);
void ifree(graph_builder_t *g, void *ptr);

static __attribute__((unused)) void L(initialize)(TYPE *vec) {
  *vec = (TYPE){NULL, 0, 0};
}

static __attribute__((unused)) void L(destroy)(graph_builder_t *g, TYPE *vec) {
#ifdef DESTRUCTOR
  for (size_t i = 0; i < vec->size; i++) {
    DESTRUCTOR(g, &vec->ary[i]);
  }
#endif

  ifree(g, vec->ary);
}

static __attribute__((unused)) CONTAINED_TYPE *
M(emplace_n)(graph_builder_t *g, TYPE *vec, size_t n, const char *api_fn_name) {
  assert(vec->size <= vec->capacity);

  if (vec->size + n > vec->capacity) {
    const size_t capacity = n + vec->capacity + (vec->capacity >> 1);
    CONTAINED_TYPE *ary =
        irealloc(g, vec->ary, sizeof(CONTAINED_TYPE), capacity, vec->capacity, api_fn_name);

    if (ary == NULL) {
      return NULL;
    }

    vec->ary = ary;
    vec->capacity = capacity;
  }

  const size_t index = vec->size;

  vec->size += n;
  assert(vec->size <= vec->capacity);

  return &vec->ary[index];
}

static __attribute__((unused)) CONTAINED_TYPE *
M(emplace)(graph_builder_t *g, TYPE *vec, const char *api_fn_name) {
  return M(emplace_n)(g, vec, 1, api_fn_name);
}

static __attribute__((unused)) int
M(push)(graph_builder_t *g, TYPE *vec, CONTAINED_TYPE value, const char *api_fn_name) {
  CONTAINED_TYPE *ptr = M(emplace)(g, vec, api_fn_name);

  if (ptr == NULL) {
    return -1;
  }

  *ptr = value;
  return 0;
}

static __attribute__((unused)) void M(pop_n)(TYPE *vec, size_t n) {
  assert(vec->size >= n);
  vec->size -= n;
}

static __attribute__((unused)) void M(pop)(TYPE *vec) {
  M(pop_n)(vec, 1);
}

#endif

#undef TYPE_ONLY
#undef METHODS_ONLY

#undef NAME
#undef CONTAINED_TYPE
#undef CONSTRUCTOR
#undef DESTRUCTOR

#undef PASTE2
#undef PASTE
#undef L
#undef M
#undef TYPE
