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
 *     claim that you wrote the original software. If you use this software in
 *     a product, an acknowledgment in the product documentation would be
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


/*
 * Tests for unique attribute names per element (docs/issues.md):
 * Reject duplicate attribute names on the same element (required by XML
 * well-formedness). Implemented in 0.11.0.
 */


/**
 * @return true iff xml string equals the C string
 */
static bool string_equals(struct xml_string* a, char const* b) {
	if (!a) {
		return b == NULL || b[0] == '\0';
	}
	size_t a_length = xml_string_length(a);
	size_t b_length = strlen(b);

	uint8_t* a_buffer = malloc((a_length + 1) * sizeof(uint8_t));
	if (!a_buffer)
		return false;
	xml_string_copy(a, a_buffer, a_length);
	a_buffer[a_length] = 0;

	bool equal = (a_length == b_length && memcmp(a_buffer, b, a_length) == 0);
	free(a_buffer);
	return equal;
}


#define SOURCE(source, content)						\
	uint8_t* source = calloc(strlen(content) + 1, sizeof(uint8_t));	\
	{	char const* content_string = content;			\
		memcpy(source, content_string, strlen(content) + 1);	\
	}


/**
 * Duplicate attribute names on the same element must be rejected.
 */
static void test_duplicate_attribute_rejected(void **state) {
	(void)state;
	SOURCE(source, "<e a=\"1\" a=\"2\">x</e>");
	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	if (document)
		xml_document_free(document, true);
	else
		free(source);
	assert_null(document);
}


/**
 * Same attribute name repeated three times.
 */
static void test_duplicate_attribute_three_times_rejected(void **state) {
	(void)state;
	SOURCE(source, "<elem id=\"a\" id=\"b\" id=\"c\">x</elem>");
	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	if (document)
		xml_document_free(document, true);
	else
		free(source);
	assert_null(document);
}


/**
 * Element with unique attribute names must parse successfully.
 */
static void test_unique_attributes_accepted(void **state) {
	(void)state;
	SOURCE(source, "<e a=\"1\" b=\"2\">x</e>");
	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	assert_non_null(document);
	if (document) {
		struct xml_node* root = xml_document_root(document);
		assert_int_equal(xml_node_attributes(root), 2);
		assert_true(string_equals(xml_node_attribute_name(root, 0), "a"));
		assert_true(string_equals(xml_node_attribute_content(root, 0), "1"));
		assert_true(string_equals(xml_node_attribute_name(root, 1), "b"));
		assert_true(string_equals(xml_node_attribute_content(root, 1), "2"));
		xml_document_free(document, true);
	} else {
		free(source);
	}
}


/**
 * Element with single attribute (no duplicate).
 */
static void test_single_attribute_accepted(void **state) {
	(void)state;
	SOURCE(source, "<node id=\"only\">content</node>");
	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	assert_non_null(document);
	if (document) {
		struct xml_node* root = xml_document_root(document);
		assert_int_equal(xml_node_attributes(root), 1);
		assert_true(string_equals(xml_node_attribute_name(root, 0), "id"));
		assert_true(string_equals(xml_node_attribute_content(root, 0), "only"));
		xml_document_free(document, true);
	} else {
		free(source);
	}
}


	static const struct CMUnitTest tests[] = {
	cmocka_unit_test(test_duplicate_attribute_rejected),
	cmocka_unit_test(test_duplicate_attribute_three_times_rejected),
	cmocka_unit_test(test_unique_attributes_accepted),
	cmocka_unit_test(test_single_attribute_accepted),
};

void get_unit_c_unique_attributes_tests(const struct CMUnitTest** out_tests, size_t* out_count) {
	*out_tests = tests;
	*out_count = sizeof(tests) / sizeof(tests[0]);
}
