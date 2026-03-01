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
 * Processing instructions <?...?> are skipped by the parser (docs/issues.md #30).
 * XML declaration <?xml ...?> is skipped like any other PI.
 */
static void test_xml_processing_instruction_xml_decl_before_root(void **state) {
	(void)state;
	SOURCE(source, "<?xml version=\"1.0\"?><root>content</root>");
	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	assert_non_null(document);
	struct xml_node* root = xml_document_root(document);
	assert_non_null(root);
	assert_true(string_equals(xml_node_name(root), "root"));
	assert_true(string_equals(xml_node_content(root), "content"));
	xml_document_free(document, true);
}


/**
 * Other PIs (not xml) before root are skipped.
 */
static void test_xml_processing_instruction_other_pi_before_root(void **state) {
	(void)state;
	SOURCE(source, "<?mypi something?><a>b</a>");
	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	assert_non_null(document);
	struct xml_node* root = xml_document_root(document);
	assert_non_null(root);
	assert_true(string_equals(xml_node_name(root), "a"));
	assert_true(string_equals(xml_node_content(root), "b"));
	xml_document_free(document, true);
}


/**
 * Multiple PIs before root (xml decl + custom PI).
 */
static void test_xml_processing_instruction_multiple_before_root(void **state) {
	(void)state;
	SOURCE(source, "<?xml version=\"1.0\"?><?stylesheet type=\"text/css\"?><r/>");
	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	assert_non_null(document);
	struct xml_node* root = xml_document_root(document);
	assert_non_null(root);
	assert_true(string_equals(xml_node_name(root), "r/"));  /* self-closing: name includes / */
	xml_document_free(document, true);
}


/**
 * PI between child nodes is skipped (like comments).
 */
static void test_xml_processing_instruction_between_children(void **state) {
	(void)state;
	SOURCE(source, "<r><?pi data?><c>x</c></r>");
	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	assert_non_null(document);
	struct xml_node* root = xml_document_root(document);
	assert_non_null(root);
	assert_true(string_equals(xml_node_name(root), "r"));
	assert_int_equal(xml_node_children(root), 1);
	struct xml_node* child = xml_node_child(root, 0);
	assert_true(string_equals(xml_node_name(child), "c"));
	assert_true(string_equals(xml_node_content(child), "x"));
	xml_document_free(document, true);
}


/**
 * PI and comment mix before root.
 */
static void test_xml_processing_instruction_and_comment_mix(void **state) {
	(void)state;
	SOURCE(source, "<?xml?><!-- comment --><root/>");
	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	assert_non_null(document);
	struct xml_node* root = xml_document_root(document);
	assert_non_null(root);
	assert_true(string_equals(xml_node_name(root), "root/"));  /* self-closing: name includes / */
	xml_document_free(document, true);
}


/**
 * Minimal PI before root.
 */
static void test_xml_processing_instruction_minimal(void **state) {
	(void)state;
	SOURCE(source, "<?x?><r/>");
	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	assert_non_null(document);
	struct xml_node* root = xml_document_root(document);
	assert_non_null(root);
	assert_true(string_equals(xml_node_name(root), "r/"));  /* self-closing: name includes / */
	xml_document_free(document, true);
}


/**
 * Multiline PI (e.g. xml decl with encoding).
 */
static void test_xml_processing_instruction_multiline(void **state) {
	(void)state;
	SOURCE(source, "<?xml version=\"1.0\"\n encoding=\"UTF-8\"?><r>ok</r>");
	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	assert_non_null(document);
	struct xml_node* root = xml_document_root(document);
	assert_non_null(root);
	assert_true(string_equals(xml_node_name(root), "r"));
	assert_true(string_equals(xml_node_content(root), "ok"));
	xml_document_free(document, true);
}


	static const struct CMUnitTest tests[] = {
	cmocka_unit_test(test_xml_processing_instruction_xml_decl_before_root),
	cmocka_unit_test(test_xml_processing_instruction_other_pi_before_root),
	cmocka_unit_test(test_xml_processing_instruction_multiple_before_root),
	cmocka_unit_test(test_xml_processing_instruction_between_children),
	cmocka_unit_test(test_xml_processing_instruction_and_comment_mix),
	cmocka_unit_test(test_xml_processing_instruction_minimal),
	cmocka_unit_test(test_xml_processing_instruction_multiline),
};

void get_unit_c_pi_tests(const struct CMUnitTest** out_tests, size_t* out_count) {
	*out_tests = tests;
	*out_count = sizeof(tests) / sizeof(tests[0]);
}
