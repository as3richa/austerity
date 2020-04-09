#ifndef ALLOC_H
#define ALLOC_H

#include "common.h"

struct allocator;

void initialize_allocator(struct allocator *a,
                          void *(*alloc)(void *, size_t),
                          void (*free)(void *, void *),
                          void *user);

void destroy_allocator(graph_builder_t *g, struct allocator *a);

void *ialloc(graph_builder_t *g, size_t size, const char *call);

void *irealloc(
    graph_builder_t *g, void *ptr, size_t elem_size, size_t size, size_t prev, const char *call);

void ifree(graph_builder_t *g, void *ptr);

char *copy_buffer(graph_builder_t *g, const char *buffer, size_t size, const char *call);

char *copy_str(graph_builder_t *g, const char *str, const char *call);

argv_t *alloc_argv(graph_builder_t *g, const char *call);

environment_t *alloc_env(graph_builder_t *g, const char *call);

func_t *alloc_func(graph_builder_t *g, const char *call);

#endif
