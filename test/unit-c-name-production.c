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
 * Tests for stricter tag names / XML Name production (docs/issues.md):
 * Reject or constrain tag names to the XML Name production (e.g. start with
 * letter, '_', or ':'; then Name characters). Parser rejects names that
 * start with a digit (e.g. <2tag>).
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
 * Tag name must not start with a digit (XML Name starts with Letter | '_' | ':').
 * Parser should reject <2tag>...</2tag>.
 */
static void test_tag_name_reject_starts_with_digit(void **state) {
	(void)state;
	SOURCE(source, "<2tag>x</2tag>");
	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	if (document)
		xml_document_free(document, true);
	else
		free(source);
	assert_null(document);
}


/**
 * Tag name starting with a digit in opening tag only (no closing tag match).
 */
static void test_tag_name_reject_starts_with_digit_self_closing(void **state) {
	(void)state;
	SOURCE(source, "<2tag/>");
	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	if (document)
		xml_document_free(document, true);
	else
		free(source);
	assert_null(document);
}


/**
 * Valid name: starts with letter. Should parse successfully.
 */
static void test_tag_name_accept_letter_start(void **state) {
	(void)state;
	SOURCE(source, "<a>x</a>");
	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	assert_non_null(document);
	if (document) {
		struct xml_node* root = xml_document_root(document);
		assert_true(string_equals(xml_node_name(root), "a"));
		xml_document_free(document, true);
	} else {
		free(source);
	}
}


/**
 * Valid name: starts with underscore (NameStartChar).
 */
static void test_tag_name_accept_underscore_start(void **state) {
	(void)state;
	SOURCE(source, "<_elem>content</_elem>");
	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	assert_non_null(document);
	if (document) {
		struct xml_node* root = xml_document_root(document);
		assert_true(string_equals(xml_node_name(root), "_elem"));
		xml_document_free(document, true);
	} else {
		free(source);
	}
}


/**
 * Valid name: starts with colon (e.g. local part of prefixed name).
 */
static void test_tag_name_accept_colon_start(void **state) {
	(void)state;
	SOURCE(source, "<:local>x</:local>");
	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	assert_non_null(document);
	if (document) {
		struct xml_node* root = xml_document_root(document);
		assert_true(string_equals(xml_node_name(root), ":local"));
		xml_document_free(document, true);
	} else {
		free(source);
	}
}


/**
 * Valid name: letter then digits (NameChar).
 */
static void test_tag_name_accept_letter_then_digits(void **state) {
	(void)state;
	SOURCE(source, "<a1>x</a1>");
	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	assert_non_null(document);
	if (document) {
		struct xml_node* root = xml_document_root(document);
		assert_true(string_equals(xml_node_name(root), "a1"));
		xml_document_free(document, true);
	} else {
		free(source);
	}
}


/**
 * Valid name: prefixed form (ns:local).
 */
static void test_tag_name_accept_prefixed(void **state) {
	(void)state;
	SOURCE(source, "<ns:local>x</ns:local>");
	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	assert_non_null(document);
	if (document) {
		struct xml_node* root = xml_document_root(document);
		assert_true(string_equals(xml_node_name(root), "ns:local"));
		xml_document_free(document, true);
	} else {
		free(source);
	}
}


	static const struct CMUnitTest tests[] = {
	cmocka_unit_test(test_tag_name_reject_starts_with_digit),
	cmocka_unit_test(test_tag_name_reject_starts_with_digit_self_closing),
	cmocka_unit_test(test_tag_name_accept_letter_start),
	cmocka_unit_test(test_tag_name_accept_underscore_start),
	cmocka_unit_test(test_tag_name_accept_colon_start),
	cmocka_unit_test(test_tag_name_accept_letter_then_digits),
	cmocka_unit_test(test_tag_name_accept_prefixed),
};

void get_unit_c_name_production_tests(const struct CMUnitTest** out_tests, size_t* out_count) {
	*out_tests = tests;
	*out_count = sizeof(tests) / sizeof(tests[0]);
}
