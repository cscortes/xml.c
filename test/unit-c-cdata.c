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
 * CDATA sections <![CDATA[...]]> (docs/issues.md #40).
 * Parser should recognize CDATA and expose its content as character data
 * (no markup or entity interpretation inside CDATA).
 */


/**
 * Simple CDATA: content is the literal string between CDATA delimiters.
 */
static void test_cdata_simple(void **state) {
	(void)state;
	SOURCE(source, "<root><![CDATA[hello]]></root>");
	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	if (!document) {
		free(source);
		assert_true(false);
		return;
	}
	struct xml_node* root = xml_document_root(document);
	assert_non_null(root);
	assert_true(string_equals(xml_node_name(root), "root"));
	assert_true(string_equals(xml_node_content(root), "hello"));
	xml_document_free(document, true);
}


/**
 * CDATA with angle brackets: < and > are not parsed as markup.
 */
static void test_cdata_angle_brackets(void **state) {
	(void)state;
	SOURCE(source, "<root><![CDATA[<tag>value</tag>]]></root>");
	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	if (!document) {
		free(source);
		assert_true(false);
		return;
	}
	struct xml_node* root = xml_document_root(document);
	assert_non_null(root);
	assert_true(string_equals(xml_node_content(root), "<tag>value</tag>"));
	xml_document_free(document, true);
}


/**
 * CDATA with ampersand: & is not interpreted as entity start.
 */
static void test_cdata_ampersand(void **state) {
	(void)state;
	SOURCE(source, "<root><![CDATA[a & b]]></root>");
	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	if (!document) {
		free(source);
		assert_true(false);
		return;
	}
	struct xml_node* root = xml_document_root(document);
	assert_non_null(root);
	assert_true(string_equals(xml_node_content(root), "a & b"));
	xml_document_free(document, true);
}


/**
 * Empty CDATA section.
 */
static void test_cdata_empty(void **state) {
	(void)state;
	SOURCE(source, "<root><![CDATA[]]></root>");
	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	if (!document) {
		free(source);
		assert_true(false);
		return;
	}
	struct xml_node* root = xml_document_root(document);
	assert_non_null(root);
	assert_true(string_equals(xml_node_content(root), ""));
	xml_document_free(document, true);
}


/**
 * CDATA in a child element.
 */
static void test_cdata_in_child(void **state) {
	(void)state;
	SOURCE(source, "<r><c><![CDATA[child data]]></c></r>");
	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	if (!document) {
		free(source);
		assert_true(false);
		return;
	}
	struct xml_node* root = xml_document_root(document);
	assert_non_null(root);
	assert_int_equal(xml_node_children(root), 1);
	struct xml_node* child = xml_node_child(root, 0);
	assert_true(string_equals(xml_node_name(child), "c"));
	assert_true(string_equals(xml_node_content(child), "child data"));
	xml_document_free(document, true);
}


/**
 * Mixed text and CDATA: character data before and after CDATA are concatenated.
 */
static void test_cdata_mixed_text(void **state) {
	(void)state;
	SOURCE(source, "<root>before<![CDATA[mid]]>after</root>");
	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	if (!document) {
		free(source);
		assert_true(false);
		return;
	}
	struct xml_node* root = xml_document_root(document);
	assert_non_null(root);
	assert_true(string_equals(xml_node_content(root), "beforemidafter"));
	xml_document_free(document, true);
}


/**
 * Two adjacent CDATA sections: content is concatenated.
 */
static void test_cdata_adjacent_sections(void **state) {
	(void)state;
	SOURCE(source, "<root><![CDATA[one]]><![CDATA[two]]></root>");
	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	if (!document) {
		free(source);
		assert_true(false);
		return;
	}
	struct xml_node* root = xml_document_root(document);
	assert_non_null(root);
	assert_true(string_equals(xml_node_content(root), "onetwo"));
	xml_document_free(document, true);
}


/**
 * CDATA with newline (whitespace preserved in CDATA).
 */
static void test_cdata_with_newline(void **state) {
	(void)state;
	SOURCE(source, "<root><![CDATA[line1\nline2]]></root>");
	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	if (!document) {
		free(source);
		assert_true(false);
		return;
	}
	struct xml_node* root = xml_document_root(document);
	assert_non_null(root);
	assert_true(string_equals(xml_node_content(root), "line1\nline2"));
	xml_document_free(document, true);
}


/**
 * Unclosed CDATA (invalid): parser should reject document (return NULL or fail safely).
 */
static void test_cdata_unclosed_invalid(void **state) {
	(void)state;
	SOURCE(source, "<root><![CDATA[no end");
	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	/* Expect parse failure: no valid way to parse unclosed CDATA */
	assert_true(document == NULL);
	if (document)
		xml_document_free(document, true);
	else
		free(source);
}


	static const struct CMUnitTest tests[] = {
	cmocka_unit_test(test_cdata_simple),
	cmocka_unit_test(test_cdata_angle_brackets),
	cmocka_unit_test(test_cdata_ampersand),
	cmocka_unit_test(test_cdata_empty),
	cmocka_unit_test(test_cdata_in_child),
	cmocka_unit_test(test_cdata_mixed_text),
	cmocka_unit_test(test_cdata_adjacent_sections),
	cmocka_unit_test(test_cdata_with_newline),
	cmocka_unit_test(test_cdata_unclosed_invalid),
};

void get_unit_c_cdata_tests(const struct CMUnitTest** out_tests, size_t* out_count) {
	*out_tests = tests;
	*out_count = sizeof(tests) / sizeof(tests[0]);
}
