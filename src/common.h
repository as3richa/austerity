#ifndef COMMON_H
#define COMMON_H

#define AUSTERITY_ABBREV
#include "austerity.h"

#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

typedef uint_least32_t st_size_t;
#define ST_SIZE_MAX UINT_LEAST32_MAX

typedef st_size_t tap_t;
#define NIL_TAP (tap_t)(-1)

#define ASSERT(cond) assert(cond)

#endif
