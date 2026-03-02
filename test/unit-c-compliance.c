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
 * Tests for XML compliance candidates (docs/issues.md):
 * - Stricter tag names (Name production)
 * - Unique attribute names per element
 * - Reject standalone `&` in content and attributes
 * - Namespace support (expose xmlns, xmlns:prefix as attributes)
 *
 * Some tests expect the parser to reject invalid input (assert_null(document)).
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
 * Stricter tag names (XML Name production)
 * docs/issues.md: "Reject or constrain tag names to the XML Name production
 * (e.g. start with letter, '_', or ':'; then Name characters). Currently the
 * parser accepts any characters until '>' or space (e.g. <2tag>)."
 * ============================================================================= */

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
	/* When stricter tag names are implemented: parsing should fail. */
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


/* =============================================================================
 * Unique attribute names per element
 * docs/issues.md: "Reject duplicate attribute names on the same element
 * (required by XML well-formedness)."
 * ============================================================================= */

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
	/* When unique-attribute check is implemented: parsing should fail. */
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


/* =============================================================================
 * Reject standalone `&` in content and attributes
 * docs/issues.md: "Treat unescaped `&` in text or attribute values as an error
 * (or require entity/character reference form)."
 * ============================================================================= */

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


/* =============================================================================
 * Namespace support
 * docs/issues.md: "Parse and expose namespace declarations (xmlns, xmlns:prefix)
 * and optionally resolve prefixed names."
 * ============================================================================= */

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
	/* Stricter tag names (Name production) */
	cmocka_unit_test(test_tag_name_reject_starts_with_digit),
	cmocka_unit_test(test_tag_name_reject_starts_with_digit_self_closing),
	cmocka_unit_test(test_tag_name_accept_letter_start),
	cmocka_unit_test(test_tag_name_accept_underscore_start),
	cmocka_unit_test(test_tag_name_accept_colon_start),
	cmocka_unit_test(test_tag_name_accept_letter_then_digits),
	cmocka_unit_test(test_tag_name_accept_prefixed),
	/* Unique attribute names per element */
	cmocka_unit_test(test_duplicate_attribute_rejected),
	cmocka_unit_test(test_duplicate_attribute_three_times_rejected),
	cmocka_unit_test(test_unique_attributes_accepted),
	cmocka_unit_test(test_single_attribute_accepted),
	/* Reject standalone & in content and attributes */
	cmocka_unit_test(test_reject_standalone_ampersand_in_content),
	cmocka_unit_test(test_reject_ampersand_at_end_of_content),
	cmocka_unit_test(test_reject_invalid_entity_in_content),
	cmocka_unit_test(test_reject_standalone_ampersand_in_attribute),
	cmocka_unit_test(test_accept_amp_entity_in_content),
	/* Namespace support (expose xmlns, xmlns:prefix) */
	cmocka_unit_test(test_namespace_default_xmlns_exposed),
	cmocka_unit_test(test_namespace_prefixed_xmlns_exposed),
	cmocka_unit_test(test_namespace_default_and_prefixed_exposed),
};

void get_unit_c_compliance_tests(const struct CMUnitTest** out_tests, size_t* out_count) {
	*out_tests = tests;
	*out_count = sizeof(tests) / sizeof(tests[0]);
}
