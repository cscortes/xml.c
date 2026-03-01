/**
 * Common macros for xml.c and its example/tests.
 * Not part of the public API.
 */
#ifndef XML_COMMON_H
#define XML_COMMON_H

#include <stddef.h>

static inline size_t MIN(size_t a, size_t b) {
	return a < b ? a : b;
}

static inline size_t MAX(size_t a, size_t b) {
	return a > b ? a : b;
}

#define UNUSED(x) ((void)(x))

#endif
