#ifndef ALLOC_H
#define ALLOC_H

#include "common.h"

void *g_alloc(graph_builder_t *g, size_t size, const char *call);

void *g_realloc(
    graph_builder_t *g, void *ptr, size_t elem_size, size_t size, size_t prev, const char *call);

void g_free(graph_builder_t *g, void *ptr);

char *g_copy_buffer(graph_builder_t *g, const char *buffer, size_t size, const char *call);

char *g_copy_str(graph_builder_t *g, const char *str, const char *call);

argv_t *g_alloc_argv(graph_builder_t *g, const char *call);

environment_t *g_alloc_env(graph_builder_t *g, const char *call);

func_t *g_alloc_func(graph_builder_t *g, const char *call);

#endif
