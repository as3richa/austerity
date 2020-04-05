#ifndef VEC_H
#define VEC_H

#define VEC_RESERVE_N(contained_type, g, vec, n, ret, api_fn_name)                                 \
  do {                                                                                             \
    assert((vec)->size <= (vec)->capacity);                                                        \
    if ((vec)->size + (n) > (vec)->capacity) {                                                     \
      const size_t elem_size = sizeof(contained_type);                                             \
      const size_t capacity = 2 * (vec)->capacity + (n);                                           \
      contained_type *ary =                                                                        \
          irealloc(g, (vec)->ary, elem_size, capacity, (vec)->capacity, (api_fn_name));            \
      if (ary == NULL) {                                                                           \
        return (ret);                                                                              \
      }                                                                                            \
      (vec)->ary = ary;                                                                            \
      (vec)->capacity = capacity;                                                                  \
    }                                                                                              \
    assert((vec)->size + (n) <= (vec)->capacity);                                                  \
  } while (0)

#endif
