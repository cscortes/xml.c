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
 * Get the tag name / Name production test array from unit-c-name-production.c
 * (stricter tag names: reject digit-start, accept letter/underscore/colon).
 *
 * @param out_tests On success, set to pointer to the test array
 * @param out_count On success, set to the number of tests
 */
void get_unit_c_name_production_tests(const struct CMUnitTest** out_tests, size_t* out_count);

/**
 * Get the unique attribute names test array from unit-c-unique-attributes.c
 * (reject duplicate attribute names per element).
 *
 * @param out_tests On success, set to pointer to the test array
 * @param out_count On success, set to the number of tests
 */
void get_unit_c_unique_attributes_tests(const struct CMUnitTest** out_tests, size_t* out_count);

/**
 * Get the standalone-ampersand rejection test array from unit-c-ampersand-reject.c
 * (reject unescaped & in content and attributes).
 *
 * @param out_tests On success, set to pointer to the test array
 * @param out_count On success, set to the number of tests
 */
void get_unit_c_ampersand_reject_tests(const struct CMUnitTest** out_tests, size_t* out_count);

/**
 * Get the namespace support test array from unit-c-namespace.c
 * (xmlns and xmlns:prefix exposed as attributes).
 *
 * @param out_tests On success, set to pointer to the test array
 * @param out_count On success, set to the number of tests
 */
void get_unit_c_namespace_tests(const struct CMUnitTest** out_tests, size_t* out_count);

/**
 * Get the entity and character reference test array from unit-c-entities.c
 * (predefined entities &amp; &lt; &gt; &quot; &apos;, decimal &#N;, hex &#xN;).
 *
 * @param out_tests On success, set to pointer to the test array
 * @param out_count On success, set to the number of tests
 */
void get_unit_c_entities_tests(const struct CMUnitTest** out_tests, size_t* out_count);

/**
 * Get the DOCTYPE/DTD test array from unit-c-doctype.c
 * (skip or parse <!DOCTYPE ...>; internal subset, no external entities).
 *
 * @param out_tests On success, set to pointer to the test array
 * @param out_count On success, set to the number of tests
 */
void get_unit_c_doctype_tests(const struct CMUnitTest** out_tests, size_t* out_count);

/**
 * Get the encoding declaration test array from unit-c-encoding.c
 * (honor encoding in <?xml ...?>; reject unsupported encodings; UTF-8 only).
 *
 * @param out_tests On success, set to pointer to the test array
 * @param out_count On success, set to the number of tests
 */
void get_unit_c_encoding_tests(const struct CMUnitTest** out_tests, size_t* out_count);

#endif
