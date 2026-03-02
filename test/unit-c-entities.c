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


/*
 * Tests for entity and character references (docs/issues.md):
 * - Predefined entities in content and attributes: &amp; &lt; &gt; &quot; &apos;
 * - Decimal character references &#N; and hexadecimal &#xN; in element and attribute content.
 *
 * Expected behavior once the feature is implemented: parser expands references
 * so that node/attribute content is the resolved character(s). These tests
 * currently fail (and are skipped for pass/fail counts) until the feature is added.
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


/* =============================================================================
 * Predefined entities in element content
 * &amp; → &   &lt; → <   &gt; → >   &quot; → "   &apos; → '
 * ============================================================================= */

static void test_entity_amp_in_content(void **state) {
	(void)state;
	SOURCE(source, "<root>&amp;</root>");
	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	if (!document) {
		free(source);
		assert_true(false);
		return;
	}
	struct xml_node* root = xml_document_root(document);
	assert_non_null(root);
	if (!string_equals(xml_node_content(root), "&")) {
		xml_document_free(document, true);
		assert_true(false);
		return;
	}
	xml_document_free(document, true);
}


static void test_entity_lt_in_content(void **state) {
	(void)state;
	SOURCE(source, "<root>&lt;</root>");
	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	if (!document) {
		free(source);
		assert_true(false);
		return;
	}
	struct xml_node* root = xml_document_root(document);
	assert_non_null(root);
	if (!string_equals(xml_node_content(root), "<")) {
		xml_document_free(document, true);
		assert_true(false);
		return;
	}
	xml_document_free(document, true);
}


static void test_entity_gt_in_content(void **state) {
	(void)state;
	SOURCE(source, "<root>&gt;</root>");
	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	if (!document) {
		free(source);
		assert_true(false);
		return;
	}
	struct xml_node* root = xml_document_root(document);
	assert_non_null(root);
	if (!string_equals(xml_node_content(root), ">")) {
		xml_document_free(document, true);
		assert_true(false);
		return;
	}
	xml_document_free(document, true);
}


static void test_entity_quot_in_content(void **state) {
	(void)state;
	SOURCE(source, "<root>&quot;</root>");
	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	if (!document) {
		free(source);
		assert_true(false);
		return;
	}
	struct xml_node* root = xml_document_root(document);
	assert_non_null(root);
	if (!string_equals(xml_node_content(root), "\"")) {
		xml_document_free(document, true);
		assert_true(false);
		return;
	}
	xml_document_free(document, true);
}


static void test_entity_apos_in_content(void **state) {
	(void)state;
	SOURCE(source, "<root>&apos;</root>");
	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	if (!document) {
		free(source);
		assert_true(false);
		return;
	}
	struct xml_node* root = xml_document_root(document);
	assert_non_null(root);
	if (!string_equals(xml_node_content(root), "'")) {
		xml_document_free(document, true);
		assert_true(false);
		return;
	}
	xml_document_free(document, true);
}


static void test_entity_mixed_in_content(void **state) {
	(void)state;
	SOURCE(source, "<root>a&amp;b&lt;c&gt;d&quot;e&apos;f</root>");
	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	if (!document) {
		free(source);
		assert_true(false);
		return;
	}
	struct xml_node* root = xml_document_root(document);
	assert_non_null(root);
	if (!string_equals(xml_node_content(root), "a&b<c>d\"e'f")) {
		xml_document_free(document, true);
		assert_true(false);
		return;
	}
	xml_document_free(document, true);
}


/* =============================================================================
 * Predefined entities in attribute values
 * ============================================================================= */

static void test_entity_amp_in_attribute(void **state) {
	(void)state;
	SOURCE(source, "<root a=\"&amp;\"></root>");
	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	if (!document) {
		free(source);
		assert_true(false);
		return;
	}
	struct xml_node* root = xml_document_root(document);
	assert_non_null(root);
	if (xml_node_attributes(root) < 1 || !string_equals(xml_node_attribute_content(root, 0), "&")) {
		xml_document_free(document, true);
		assert_true(false);
		return;
	}
	xml_document_free(document, true);
}


static void test_entity_lt_gt_in_attribute(void **state) {
	(void)state;
	SOURCE(source, "<root x=\"&lt;tag&gt;\"></root>");
	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	if (!document) {
		free(source);
		assert_true(false);
		return;
	}
	struct xml_node* root = xml_document_root(document);
	assert_non_null(root);
	if (!string_equals(xml_node_attribute_content(root, 0), "<tag>")) {
		xml_document_free(document, true);
		assert_true(false);
		return;
	}
	xml_document_free(document, true);
}


static void test_entity_quot_apos_in_attribute(void **state) {
	(void)state;
	/* Attribute delimited by double quotes; value contains &quot; and &apos; */
	SOURCE(source, "<root a=\"&quot;hello&apos;world&quot;\"></root>");
	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	if (!document) {
		free(source);
		assert_true(false);
		return;
	}
	struct xml_node* root = xml_document_root(document);
	assert_non_null(root);
	if (!string_equals(xml_node_attribute_content(root, 0), "\"hello'world\"")) {
		xml_document_free(document, true);
		assert_true(false);
		return;
	}
	xml_document_free(document, true);
}


/* =============================================================================
 * Decimal character references &#N; in element content
 * ============================================================================= */

static void test_char_ref_decimal_content(void **state) {
	(void)state;
	/* &#65; = 'A', &#97; = 'a' */
	SOURCE(source, "<root>&#65;&#97;</root>");
	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	if (!document) {
		free(source);
		assert_true(false);
		return;
	}
	struct xml_node* root = xml_document_root(document);
	assert_non_null(root);
	if (!string_equals(xml_node_content(root), "Aa")) {
		xml_document_free(document, true);
		assert_true(false);
		return;
	}
	xml_document_free(document, true);
}


static void test_char_ref_decimal_space(void **state) {
	(void)state;
	/* &#32; = space */
	SOURCE(source, "<root>a&#32;b</root>");
	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	if (!document) {
		free(source);
		assert_true(false);
		return;
	}
	struct xml_node* root = xml_document_root(document);
	assert_non_null(root);
	if (!string_equals(xml_node_content(root), "a b")) {
		xml_document_free(document, true);
		assert_true(false);
		return;
	}
	xml_document_free(document, true);
}


/* =============================================================================
 * Hexadecimal character references &#xN; in element content
 * ============================================================================= */

static void test_char_ref_hex_content(void **state) {
	(void)state;
	/* &#x41; = 'A', &#x61; = 'a' (case insensitive in XML) */
	SOURCE(source, "<root>&#x41;&#x61;</root>");
	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	if (!document) {
		free(source);
		assert_true(false);
		return;
	}
	struct xml_node* root = xml_document_root(document);
	assert_non_null(root);
	if (!string_equals(xml_node_content(root), "Aa")) {
		xml_document_free(document, true);
		assert_true(false);
		return;
	}
	xml_document_free(document, true);
}


static void test_char_ref_hex_uppercase(void **state) {
	(void)state;
	SOURCE(source, "<root>&#x4F;K</root>");
	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	if (!document) {
		free(source);
		assert_true(false);
		return;
	}
	struct xml_node* root = xml_document_root(document);
	assert_non_null(root);
	if (!string_equals(xml_node_content(root), "OK")) {
		xml_document_free(document, true);
		assert_true(false);
		return;
	}
	xml_document_free(document, true);
}


/* =============================================================================
 * Character references in attribute values
 * ============================================================================= */

static void test_char_ref_decimal_in_attribute(void **state) {
	(void)state;
	SOURCE(source, "<root id=\"&#65;&#66;&#67;\"></root>");
	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	if (!document) {
		free(source);
		assert_true(false);
		return;
	}
	struct xml_node* root = xml_document_root(document);
	assert_non_null(root);
	if (!string_equals(xml_node_attribute_content(root, 0), "ABC")) {
		xml_document_free(document, true);
		assert_true(false);
		return;
	}
	xml_document_free(document, true);
}


static void test_char_ref_hex_in_attribute(void **state) {
	(void)state;
	SOURCE(source, "<root h=\"&#x48;i\"></root>");
	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	if (!document) {
		free(source);
		assert_true(false);
		return;
	}
	struct xml_node* root = xml_document_root(document);
	assert_non_null(root);
	if (!string_equals(xml_node_attribute_content(root, 0), "Hi")) {
		xml_document_free(document, true);
		assert_true(false);
		return;
	}
	xml_document_free(document, true);
}


/* =============================================================================
 * Mixed: entities and character references together
 * ============================================================================= */

static void test_entity_and_char_ref_mixed_content(void **state) {
	(void)state;
	SOURCE(source, "<root>&lt;&#32;&amp;&#32;&gt;</root>");
	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	if (!document) {
		free(source);
		assert_true(false);
		return;
	}
	struct xml_node* root = xml_document_root(document);
	assert_non_null(root);
	if (!string_equals(xml_node_content(root), "< & >")) {
		xml_document_free(document, true);
		assert_true(false);
		return;
	}
	xml_document_free(document, true);
}


	static const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_entity_amp_in_content),
		cmocka_unit_test(test_entity_lt_in_content),
		cmocka_unit_test(test_entity_gt_in_content),
		cmocka_unit_test(test_entity_quot_in_content),
		cmocka_unit_test(test_entity_apos_in_content),
		cmocka_unit_test(test_entity_mixed_in_content),
		cmocka_unit_test(test_entity_amp_in_attribute),
		cmocka_unit_test(test_entity_lt_gt_in_attribute),
		cmocka_unit_test(test_entity_quot_apos_in_attribute),
		cmocka_unit_test(test_char_ref_decimal_content),
		cmocka_unit_test(test_char_ref_decimal_space),
		cmocka_unit_test(test_char_ref_hex_content),
		cmocka_unit_test(test_char_ref_hex_uppercase),
		cmocka_unit_test(test_char_ref_decimal_in_attribute),
		cmocka_unit_test(test_char_ref_hex_in_attribute),
		cmocka_unit_test(test_entity_and_char_ref_mixed_content),
	};

void get_unit_c_entities_tests(const struct CMUnitTest** out_tests, size_t* out_count) {
	*out_tests = tests;
	*out_count = sizeof(tests) / sizeof(tests[0]);
}
