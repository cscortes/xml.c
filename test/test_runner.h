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
 *
 * @param out_tests On success, set to pointer to the test array
 * @param out_count On success, set to the number of tests
 */
void get_unit_c_tests(const struct CMUnitTest** out_tests, size_t* out_count);

/**
 * Get the NULL-dereference test array from unit-c-null.c.
 *
 * @param out_tests On success, set to pointer to the test array
 * @param out_count On success, set to the number of tests
 */
void get_unit_c_null_tests(const struct CMUnitTest** out_tests, size_t* out_count);

/**
 * Get the processing-instruction test array from unit-c-pi.c.
 *
 * @param out_tests On success, set to pointer to the test array
 * @param out_count On success, set to the number of tests
 */
void get_unit_c_pi_tests(const struct CMUnitTest** out_tests, size_t* out_count);

/**
 * Get the CDATA section test array from unit-c-cdata.c.
 *
 * @param out_tests On success, set to pointer to the test array
 * @param out_count On success, set to the number of tests
 */
void get_unit_c_cdata_tests(const struct CMUnitTest** out_tests, size_t* out_count);

/**
 * Get the XML compliance test array from unit-c-compliance.c
 * (stricter tag names / Name production, unique attribute names per element).
 *
 * @param out_tests On success, set to pointer to the test array
 * @param out_count On success, set to the number of tests
 */
void get_unit_c_compliance_tests(const struct CMUnitTest** out_tests, size_t* out_count);

#endif
