/**
 * Copyright (c) 2012 ooxi/xml.c
 *     https://github.com/ooxi/xml.c
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from the
 * use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 *  1. The origin of this software must not be misrepresented; you must not
 *     claim that you wrote the original software.
 *
 *  2. Altered source versions must be plainly marked as such.
 *
 *  3. This notice may not be removed or altered from any source distribution.
 *
 * Wrapper for realloc used only when linking tests with -Wl,--wrap=realloc.
 * Allows tests to make realloc fail (return NULL) via cmocka will_return_ptr()
 * to exercise OOM/error paths without leaking or double-freeing.
 */
#include <stddef.h>
#include <cmocka.h>

/* Provided by the linker when using --wrap=realloc */
extern void* __real_realloc(void* ptr, size_t size);

void* __wrap_realloc(void* ptr, size_t size)
{
	if (has_mock())
		return mock_ptr_type(void*);
	return __real_realloc(ptr, size);
}
