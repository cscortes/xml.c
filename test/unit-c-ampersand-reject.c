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
 * Tests for rejecting standalone `&` in content and attributes (docs/issues.md):
 * Unescaped `&` in text or attribute values causes parse failure; entity or
 * character reference is required. Implemented via entity expansion; tests in 0.12.1.
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
 * Standalone `&` in element content must be rejected (not followed by entity/char ref).
 */
static void test_reject_standalone_ampersand_in_content(void **state) {
	(void)state;
	SOURCE(source, "<root>foo & bar</root>");
	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	if (document)
		xml_document_free(document, true);
	else
		free(source);
	assert_null(document);
}


/**
 * Standalone `&` at end of content must be rejected.
 */
static void test_reject_ampersand_at_end_of_content(void **state) {
	(void)state;
	SOURCE(source, "<root>x&</root>");
	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	if (document)
		xml_document_free(document, true);
	else
		free(source);
	assert_null(document);
}


/**
 * Invalid entity reference (not one of predefined/char ref) in content must be rejected.
 */
static void test_reject_invalid_entity_in_content(void **state) {
	(void)state;
	SOURCE(source, "<root>&unknown;</root>");
	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	if (document)
		xml_document_free(document, true);
	else
		free(source);
	assert_null(document);
}


/**
 * Standalone `&` in attribute value must be rejected.
 */
static void test_reject_standalone_ampersand_in_attribute(void **state) {
	(void)state;
	SOURCE(source, "<e a=\"foo & bar\">x</e>");
	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	if (document)
		xml_document_free(document, true);
	else
		free(source);
	assert_null(document);
}


/**
 * Valid use of `&` via &amp; in content must still parse.
 */
static void test_accept_amp_entity_in_content(void **state) {
	(void)state;
	SOURCE(source, "<root>a&amp;b</root>");
	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	assert_non_null(document);
	if (document) {
		struct xml_node* root = xml_document_root(document);
		assert_true(string_equals(xml_node_content(root), "a&b"));
		xml_document_free(document, true);
	} else {
		free(source);
	}
}


/**
 * Invalid entity in attribute (e.g. &bad;) must be rejected.
 */
static void test_reject_invalid_entity_in_attribute(void **state) {
	(void)state;
	SOURCE(source, "<root a=\"&bad;\"/>");
	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	if (document) {
		xml_document_free(document, true);
		fail_msg("%s", "Expected parse failure for invalid entity in attribute");
	}
	free(source);
	assert_null(document);
}


	static const struct CMUnitTest tests[] = {
	cmocka_unit_test(test_reject_standalone_ampersand_in_content),
	cmocka_unit_test(test_reject_ampersand_at_end_of_content),
	cmocka_unit_test(test_reject_invalid_entity_in_content),
	cmocka_unit_test(test_reject_standalone_ampersand_in_attribute),
	cmocka_unit_test(test_accept_amp_entity_in_content),
	cmocka_unit_test(test_reject_invalid_entity_in_attribute),
};

void get_unit_c_ampersand_reject_tests(const struct CMUnitTest** out_tests, size_t* out_count) {
	*out_tests = tests;
	*out_count = sizeof(tests) / sizeof(tests[0]);
}
