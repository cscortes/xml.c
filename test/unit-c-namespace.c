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
 * Tests for namespace support (docs/issues.md):
 * xmlns and xmlns:prefix are parsed and exposed as normal attributes;
 * no separate namespace API or prefixed-name resolution.
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
 * Default namespace xmlns="..." should parse and be exposed as an attribute.
 */
static void test_namespace_default_xmlns_exposed(void **state) {
	(void)state;
	SOURCE(source, "<root xmlns=\"http://example.com\">x</root>");
	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	assert_non_null(document);
	if (document) {
		struct xml_node* root = xml_document_root(document);
		assert_int_equal(xml_node_attributes(root), 1);
		assert_true(string_equals(xml_node_attribute_name(root, 0), "xmlns"));
		assert_true(string_equals(xml_node_attribute_content(root, 0), "http://example.com"));
		xml_document_free(document, true);
	} else {
		free(source);
	}
}


/**
 * Prefixed namespace xmlns:prefix="..." should parse and be exposed as an attribute.
 */
static void test_namespace_prefixed_xmlns_exposed(void **state) {
	(void)state;
	SOURCE(source, "<root xmlns:pre=\"http://example.com/ns\">x</root>");
	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	assert_non_null(document);
	if (document) {
		struct xml_node* root = xml_document_root(document);
		assert_int_equal(xml_node_attributes(root), 1);
		assert_true(string_equals(xml_node_attribute_name(root, 0), "xmlns:pre"));
		assert_true(string_equals(xml_node_attribute_content(root, 0), "http://example.com/ns"));
		xml_document_free(document, true);
	} else {
		free(source);
	}
}


/**
 * Both default and prefixed xmlns on same element.
 */
static void test_namespace_default_and_prefixed_exposed(void **state) {
	(void)state;
	SOURCE(source, "<root xmlns=\"http://default\" xmlns:pre=\"http://pre\">x</root>");
	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	assert_non_null(document);
	if (document) {
		struct xml_node* root = xml_document_root(document);
		assert_int_equal(xml_node_attributes(root), 2);
		/* Order may vary; check both names and values. */
		bool has_default = false;
		bool has_pre = false;
		for (size_t i = 0; i < 2; i++) {
			if (string_equals(xml_node_attribute_name(root, i), "xmlns")
			    && string_equals(xml_node_attribute_content(root, i), "http://default"))
				has_default = true;
			if (string_equals(xml_node_attribute_name(root, i), "xmlns:pre")
			    && string_equals(xml_node_attribute_content(root, i), "http://pre"))
				has_pre = true;
		}
		assert_true(has_default);
		assert_true(has_pre);
		xml_document_free(document, true);
	} else {
		free(source);
	}
}


	static const struct CMUnitTest tests[] = {
	cmocka_unit_test(test_namespace_default_xmlns_exposed),
	cmocka_unit_test(test_namespace_prefixed_xmlns_exposed),
	cmocka_unit_test(test_namespace_default_and_prefixed_exposed),
};

void get_unit_c_namespace_tests(const struct CMUnitTest** out_tests, size_t* out_count) {
	*out_tests = tests;
	*out_count = sizeof(tests) / sizeof(tests[0]);
}
