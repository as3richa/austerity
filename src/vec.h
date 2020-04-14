#include "allocator.h"
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

static void L(shallow_destroy)(TYPE *vec, allocator_t *alc) {
  a_free(alc, vec->ary);
}

static __attribute__((unused)) void L(destroy)(TYPE *vec, allocator_t *alc) {
#ifdef DESTRUCTOR
  for (size_t i = 0; i < vec->size; i++) {
    DESTRUCTOR(&vec->ary[i], alc);
  }
#endif

  L(shallow_destroy)(vec, alc);
}

static __attribute__((unused)) void L(clear)(TYPE *vec) {
  vec->size = 0;
}

static __attribute__((unused)) CONTAINED_TYPE *L(move_from)(TYPE *vec, size_t *size) {
  *size = vec->size;
  CONTAINED_TYPE *ary = vec->ary;
  L(initialize)(vec);
  return ary;
}

static __attribute__((unused)) int
M(reserve)(TYPE *vec, size_t n, allocator_t *alc, errors_t *errors) {
  ASSERT(vec->size <= vec->capacity);

  if (vec->size + n > vec->capacity) {
    const size_t capacity = n + vec->capacity + (vec->capacity >> 1);
    CONTAINED_TYPE *ary =
        a_realloc(alc, vec->ary, sizeof(CONTAINED_TYPE), capacity, vec->capacity, errors);

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
M(emplace_n)(TYPE *vec, size_t n, allocator_t *alc, errors_t *errors) {
  if (M(reserve(vec, n, alc, errors)) < 0) {
    return NULL;
  }

  ASSERT(vec->size + n <= vec->capacity);

  const size_t index = vec->size;
  vec->size += n;

  return &vec->ary[index];
}

static __attribute__((unused)) CONTAINED_TYPE *
M(emplace)(TYPE *vec, allocator_t *alc, errors_t *errors) {
  return M(emplace_n)(vec, 1, alc, errors);
}

static __attribute__((unused)) void M(unemplace)(TYPE *vec) {
  assert(vec->size > 0);
  vec->size--;
}

static __attribute__((unused)) int
M(push)(TYPE *vec, CONTAINED_TYPE value, allocator_t *alc, errors_t *errors) {
  CONTAINED_TYPE *ptr = M(emplace)(vec, alc, errors);

  if (ptr == NULL) {
    return -1;
  }

  *ptr = value;
  return 0;
}

static __attribute__((unused)) void M(pop_n)(TYPE *vec, size_t n, allocator_t *alc) {
  ASSERT(vec->size >= n);

#ifdef DESTRUCTOR
  for (size_t i = 1; i <= n; i++) {
    DESTRUCTOR(&vec->ary[vec->size - i], alc);
  }
#else
  (void)alc;
#endif

  vec->size -= n;
}

static __attribute__((unused)) int
M(extend)(TYPE *vec, size_t n, CONTAINED_TYPE value, allocator_t *alc, errors_t *errors) {
  if (n <= vec->size) {
    return 0;
  }

  const size_t delta = n - vec->size;

  if (M(reserve)(vec, delta, alc, errors) < 0) {
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
L(shallow_copy)(TYPE *dest, const TYPE *source, allocator_t *alc, errors_t *errors) {
  const size_t size = source->size;

  dest->ary = a_alloc(alc, sizeof(CONTAINED_TYPE) * size, errors);

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
