#include "alloc.h"
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

static __attribute__((unused)) void L(initialize)(TYPE *vec) {
  *vec = (TYPE){NULL, 0, 0};
}

static void L(shallow_destroy)(graph_builder_t *g, TYPE *vec) {
  g_free(g, vec->ary);
}

static __attribute__((unused)) void L(destroy)(graph_builder_t *g, TYPE *vec) {
#ifdef DESTRUCTOR
  for (size_t i = 0; i < vec->size; i++) {
    DESTRUCTOR(g, &vec->ary[i]);
  }
#endif

  L(shallow_destroy)(g, vec);
}

static __attribute__((unused)) CONTAINED_TYPE *L(move_from)(TYPE *vec, size_t *size) {
  *size = vec->size;
  CONTAINED_TYPE *ary = vec->ary;
  L(initialize)(vec);
  return ary;
}

static __attribute__((unused)) int
M(reserve)(graph_builder_t *g, TYPE *vec, size_t n, const char *call) {
  ASSERT(vec->size <= vec->capacity);

  if (vec->size + n > vec->capacity) {
    const size_t capacity = n + vec->capacity + (vec->capacity >> 1);
    CONTAINED_TYPE *ary =
        g_realloc(g, vec->ary, sizeof(CONTAINED_TYPE), capacity, vec->capacity, call);

    if (ary == NULL) {
      return -1;
    }

    vec->ary = ary;
    vec->capacity = capacity;
  }

  ASSERT(vec->size + n <= vec->capacity);
  return 0;
}

static __attribute__((unused)) CONTAINED_TYPE *
M(emplace_n)(graph_builder_t *g, TYPE *vec, size_t n, const char *call) {
  if (M(reserve(g, vec, n, call)) < 0) {
    return NULL;
  }

  ASSERT(vec->size + n <= vec->capacity);

  const size_t index = vec->size;
  vec->size += n;

  return &vec->ary[index];
}

static __attribute__((unused)) CONTAINED_TYPE *
M(emplace)(graph_builder_t *g, TYPE *vec, const char *call) {
  return M(emplace_n)(g, vec, 1, call);
}

static __attribute__((unused)) void M(unemplace)(TYPE *vec) {
  assert(vec->size > 0);
  vec->size--;
}

static __attribute__((unused)) int
M(push)(graph_builder_t *g, TYPE *vec, CONTAINED_TYPE value, const char *call) {
  CONTAINED_TYPE *ptr = M(emplace)(g, vec, call);

  if (ptr == NULL) {
    return -1;
  }

  *ptr = value;
  return 0;
}

static __attribute__((unused)) void M(pop_n)(graph_builder_t *g, TYPE *vec, size_t n) {
  ASSERT(vec->size >= n);

#ifdef DESTRUCTOR
  for (size_t i = 1; i <= n; i++) {
    DESTRUCTOR(g, &vec->ary[vec->size - i]);
  }
#else
  (void)g;
#endif

  vec->size -= n;
}

static __attribute__((unused)) int
M(extend)(graph_builder_t *g, TYPE *vec, size_t n, CONTAINED_TYPE value, const char *call) {
  if (n <= vec->size) {
    return 0;
  }

  const size_t delta = n - vec->size;

  if (M(reserve)(g, vec, delta, call) < 0) {
    return -1;
  }

  for (size_t i = 0; i < delta; i++) {
    vec->ary[vec->size + i] = value;
  }

  vec->size = n;
  return 0;
}

static __attribute__((unused)) CONTAINED_TYPE *M(ref)(TYPE *vec, size_t i) {
  ASSERT(i < vec->size);
  return &vec->ary[i];
}

static __attribute__((unused)) CONTAINED_TYPE const *M(const_ref)(const TYPE *vec, size_t i) {
  ASSERT(i < vec->size);
  return &vec->ary[i];
}

static __attribute__((unused)) int
L(shallow_copy)(graph_builder_t *g, TYPE *dest, const TYPE *source, const char *call) {
  const size_t size = source->size;

  dest->ary = g_alloc(g, sizeof(CONTAINED_TYPE) * size, call);

  if (dest->ary == NULL) {
    return -1;
  }

  memcpy(dest->ary, source->ary, sizeof(CONTAINED_TYPE) * size);
  dest->capacity = size;
  dest->size = size;

  return 0;
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
