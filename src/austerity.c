#define AUSTERITY_ABBREV

#include "austerity.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

typedef uint_least32_t st_size_t;

typedef enum {
  TT_FD_SOURCE,
  TT_FILE_SOURCE,
  TT_STDIO_SOURCE,
  TT_STR_SOURCE,
  TT_BUFFER_SOURCE,
  TT_STATIC_BUFFER_SOURCE,
  TT_FD_SINK,
  TT_FILE_SINK,
  TT_STDIO_SINK,
  TT_COMMAND,
  TT_FUNCTION
} transformer_type_t;

typedef struct {
  transformer_type_t type;

  union {
    int fd;

    struct {
      const char *path;
      int flags;
    } file;

    FILE *stdio_file;

    struct {
      char *data;
      st_size_t size;
    } buffer;

    struct {
      const char *data;
      st_size_t size;
    } static_buffer;

    struct {
      command_builder_t *cmd;
      source_t stdin;
    } command;

    struct {
      function_builder_t *fn;
      source_t *sources;
    } function;
  } data;

  struct transformer *next;
} transformer_t;

typedef struct {
  st_size_t tf_index;
  st_size_t out_index;
  st_size_t degree;
  unsigned int pathy : 1;
} source_data_t;

struct austerity_command_builder {
  int x;
};
struct austerity_function_builder {
  int x;
};

struct austerity_graph_builder {
  unsigned int panicky : 1;

  struct {
    transformer_t *ary;
    st_size_t size;
    st_size_t capacity;
  } transformers;

  struct {
    source_data_t *ary;
    st_size_t size;
    st_size_t capacity;
  } source_data;

  struct command_builder_list {
    command_builder_t cmd;
    struct command_builder_list *next;
  } * cmds;

  struct function_builder_list {
    function_builder_t fn;
    struct function_builder_list *next;
  } * fns;

  struct {
    void *(*alloc)(size_t, void *);
    void (*free)(void *, void *);
    void *user;
  } a;
};

#define ALLOC(type, g) (type *)((g)->a.alloc(sizeof(type), (g)->a.user))
#define ALLOC_N(type, n, g) (type *)((g)->a.alloc(sizeof(type) * (n), (g)->a.user))
#define FREE(ptr, g) ((g)->a.free(ptr, (g)->a.user))

#define CHECK_ALLOC(ptr, g, r)                                                                     \
  do {                                                                                             \
    if ((ptr) == NULL) {                                                                           \
      if ((g)->panicky) {                                                                          \
        abort(); /* FIXME: error message(?) */                                                     \
      }                                                                                            \
      return (r);                                                                                  \
    }                                                                                              \
  } while (0)

static void *default_alloc(size_t size, void *user) {
  (void)user;
  return malloc(size);
}

static void default_free(void *ptr, void *user) {
  (void)user;
  free(ptr);
}

static source_t create_sources(graph_builder_t *g, size_t tf_index, size_t n);

static transformer_t *push_transformer(source_t *first, graph_builder_t *g, size_t n_out) {
  assert(g->transformers.size <= g->transformers.capacity);

  if (g->transformers.size == g->transformers.capacity) {
    const size_t capacity = 2 * g->transformers.capacity + 16;

    transformer_t *ary = ALLOC_N(transformer_t, capacity, g);
    CHECK_ALLOC(ary, g, NULL);

    memcpy(ary, g->transformers.ary, sizeof(transformer_t) * g->transformers.capacity);
    FREE(g->transformers.ary, g);

    g->transformers.ary = ary;
    g->transformers.capacity = capacity;
  }

  assert(g->transformers.size < g->transformers.capacity);

  const source_t sources = create_sources(g, g->transformers.size, n_out);

  if (sources == DEV_NULL) {
    return NULL;
  }

  *first = sources;
  return &g->transformers.ary[g->transformers.size++];
}

static source_t create_sources(graph_builder_t *g, size_t tf_index, size_t n) {
  assert(g->source_data.size <= g->source_data.capacity);

  if (g->source_data.size + n > g->source_data.capacity) {
    const size_t capacity = 2 * g->transformers.capacity + n;

    source_data_t *ary = ALLOC_N(source_data_t, capacity, g);
    CHECK_ALLOC(ary, g, DEV_NULL);

    memcpy(ary, g->source_data.ary, sizeof(source_data_t) * g->source_data.capacity);
    FREE(g->source_data.ary, g);

    g->source_data.ary = ary;
    g->source_data.capacity = capacity;
  }

  assert(g->source_data.size + n <= g->source_data.capacity);

  for (size_t i = 0; i < n; i++) {
    source_data_t *data = &g->source_data.ary[g->source_data.size + i];

    data->tf_index = tf_index;
    data->out_index = i;
    data->degree = 0;
    data->pathy = 0;
  }

  const size_t result = g->source_data.size;

  g->source_data.size += n;

  return result;
}

static char *copy_buffer(const char *buffer, size_t size, graph_builder_t *g) {
  char *copy = ALLOC_N(char, size, g);
  CHECK_ALLOC(copy, g, NULL);

  memcpy(copy, buffer, size);

  return copy;
}

static char *copy_str(const char *str, graph_builder_t *g) {
  return copy_buffer(str, strlen(str) + 1, g);
}

graph_builder_t *create_graph_builder(int panicky) {
  return create_graph_builder_a(panicky, default_alloc, default_free, NULL);
}

graph_builder_t *create_graph_builder_a(int panicky,
                                        void *(*alloc)(size_t, void *),
                                        void (*free)(void *, void *),
                                        void *user) {
  graph_builder_t *g = (*alloc)(sizeof(graph_builder_t), user);

  if (g == NULL) {
    return NULL;
  }

  g->panicky = panicky;

  g->source_data.ary = NULL;
  g->source_data.size = 0;
  g->source_data.capacity = 0;

  g->transformers.ary = NULL;
  g->transformers.size = 0;
  g->transformers.capacity = 0;

  g->cmds = NULL;

  g->fns = NULL;

  g->a.alloc = alloc;
  g->a.free = free;
  g->a.user = user;

  return g;
}

void destroy_graph_builder(graph_builder_t *g) {
  FREE(g->source_data.ary, g);

  FREE(g->transformers.ary, g);

  for (struct command_builder_list *it = g->cmds, *next; it != NULL; it = next) {
    next = it->next;
    FREE(it, g);
  }

  for (struct function_builder_list *it = g->fns, *next; it != NULL; it = next) {
    next = it->next;
    FREE(it, g);
  }

  FREE(g, g);
}

source_t fd_source(graph_builder_t *g, int fd) {
  source_t out;
  transformer_t *t = push_transformer(&out, g, 1);
  t->type = TT_FD_SOURCE;
  t->data.fd = fd;
  return out;
}

source_t file_source(graph_builder_t *g, const char *path, int flags) {
  const char *my_path = copy_str(path, g);
  CHECK_ALLOC(my_path, g, DEV_NULL);

  source_t out;
  transformer_t *t = push_transformer(&out, g, 1);
  t->type = TT_FILE_SOURCE;
  t->data.file.path = my_path;
  t->data.file.flags = flags;

  return out;
}

source_t stdio_source(graph_builder_t *g, FILE *file) {
  source_t out;
  transformer_t *t = push_transformer(&out, g, 1);
  t->type = TT_STDIO_SOURCE;
  t->data.stdio_file = file;
  return out;
}

source_t str_source(graph_builder_t *g, const char *str) {
  return buffer_source(g, str, strlen(str));
}

source_t buffer_source(graph_builder_t *g, const char *data, size_t size) {
  char *my_data = copy_buffer(data, size, g);
  CHECK_ALLOC(my_data, g, DEV_NULL);

  source_t out;
  transformer_t *t = push_transformer(&out, g, 1);
  t->type = TT_BUFFER_SOURCE;
  t->data.buffer.data = my_data;
  t->data.buffer.size = size;

  return out;
}

source_t static_str_source(graph_builder_t *g, const char *str) {
  return static_buffer_source(g, str, strlen(str));
}

source_t static_buffer_source(graph_builder_t *g, const char *data, size_t size) {
  source_t out;
  transformer_t *t = push_transformer(&out, g, 1);
  t->type = TT_STATIC_BUFFER_SOURCE;
  t->data.static_buffer.data = data;
  t->data.static_buffer.size = size;
  return out;
}
