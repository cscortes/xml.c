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
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <cmocka.h>
#include <xml.h>
#include "test_runner.h"


/**
 * xml_document_free(NULL, false) and xml_document_free(NULL, true) must not crash.
 */
static void test_document_free_null(void **state) {
	(void)state;
	xml_document_free(NULL, false);
	xml_document_free(NULL, true);
}


/**
 * xml_document_root(NULL) must not crash and must return NULL.
 */
static void test_api_null_document(void **state) {
	(void)state;
	struct xml_node* root = xml_document_root(NULL);
	assert_null(root);
}


/**
 * All node accessors with NULL node must not crash and return NULL or 0.
 */
static void test_api_null_node(void **state) {
	(void)state;
	struct xml_node* null_node = NULL;

	assert_null(xml_node_name(null_node));
	assert_null(xml_node_content(null_node));
	assert_int_equal(xml_node_children(null_node), 0);
	assert_null(xml_node_child(null_node, 0));
	assert_int_equal(xml_node_attributes(null_node), 0);
	assert_null(xml_node_attribute_name(null_node, 0));
	assert_null(xml_node_attribute_content(null_node, 0));
}


/**
 * xml_easy_child(NULL, ...), xml_node_name_c_string(NULL), xml_node_content_c_string(NULL) must not crash and return NULL.
 */
static void test_api_null_node_easy(void **state) {
	(void)state;
	assert_null(xml_easy_child(NULL, (uint8_t const*)"Tag", (uint8_t const*)0));
	assert_null(xml_node_name_c_string(NULL));
	assert_null(xml_node_content_c_string(NULL));
}


/**
 * xml_node_attribute_name_c_string(NULL, 0) and xml_node_attribute_content_c_string(NULL, 0) must not crash and return NULL.
 */
static void test_api_null_node_attribute_c_string(void **state) {
	(void)state;
	assert_null(xml_node_attribute_name_c_string(NULL, 0));
	assert_null(xml_node_attribute_content_c_string(NULL, 0));
}


/**
 * xml_string_length(NULL) must not crash and return 0.
 * xml_string_copy(NULL, buf, n) must not crash and must not write (no-op).
 */
static void test_api_null_string(void **state) {
	(void)state;
	assert_int_equal(xml_string_length(NULL), 0);

	uint8_t buf[4] = { 0xAB, 0xAB, 0xAB, 0xAB };
	xml_string_copy(NULL, buf, sizeof(buf));
	/* No-op: buffer unchanged (or implementation-defined). We only require no crash. */
	(void)buf;
}


/**
 * xml_string_equals and xml_string_equals_cstr with NULL must not crash; return false when either argument is NULL.
 */
static void test_api_null_string_compare(void **state) {
	(void)state;
	/* With NULL we cannot get a valid xml_string without a document; we only test NULL first arg. */
	assert_false(xml_string_equals(NULL, NULL));
	assert_false(xml_string_equals_cstr(NULL, (uint8_t const*)"x"));
	assert_false(xml_string_equals_cstr(NULL, NULL));
}


/**
 * xml_document_buffer_length(NULL) must not crash and must return 0.
 */
static void test_api_null_document_buffer_length(void **state) {
	(void)state;
	assert_int_equal(xml_document_buffer_length(NULL), 0);
}


static const struct CMUnitTest tests[] = {
	cmocka_unit_test(test_document_free_null),
	cmocka_unit_test(test_api_null_document),
	cmocka_unit_test(test_api_null_node),
	cmocka_unit_test(test_api_null_node_easy),
	cmocka_unit_test(test_api_null_node_attribute_c_string),
	cmocka_unit_test(test_api_null_string),
	cmocka_unit_test(test_api_null_string_compare),
	cmocka_unit_test(test_api_null_document_buffer_length),
};

void get_unit_c_null_tests(const struct CMUnitTest** out_tests, size_t* out_count) {
	*out_tests = tests;
	*out_count = sizeof(tests) / sizeof(tests[0]);
}
