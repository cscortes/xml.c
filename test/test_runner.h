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
 *     claim that you wrote the original software. If you use this software in a
 *     product, an acknowledgment in the product documentation would be
 *     appreciated but is not required.
 *
 *  2. Altered source versions must be plainly marked as such, and must not be
 *     misrepresented as being the original software.
 *
 *  3. This notice may not be removed or altered from any source distribution.
 */
#ifndef TEST_RUNNER_H
#define TEST_RUNNER_H

#include <stddef.h>
#include <cmocka.h>

/**
 * Get the parsing/API-success test array from unit-c.c.
 */
void get_unit_c_tests(const struct CMUnitTest** out_tests, size_t* out_count);

/**
 * Get the NULL-dereference test array from unit-c-null.c.
 */
void get_unit_c_null_tests(const struct CMUnitTest** out_tests, size_t* out_count);

#endif
