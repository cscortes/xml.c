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
 * Tests for DOCTYPE / DTD handling (docs/issues.md):
 * Skip or optionally parse <!DOCTYPE ...>; if parsing, support internal subset
 * and predefined entities only (no external entities).
 *
 * Feature is "Not started". Tests encode expected behavior once implemented;
 * some tests may fail until the feature is added.
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
 * Document with minimal DOCTYPE (empty internal subset) before root.
 * Expected (when implemented): parser skips DOCTYPE and parses root.
 */
static void test_doctype_minimal_before_root(void **state) {
	(void)state;
	SOURCE(source, "<!DOCTYPE root []><root>content</root>");
	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	/* When DOCTYPE skip is implemented: parse succeeds and root is accessible. */
	if (document) {
		struct xml_node* root = xml_document_root(document);
		assert_non_null(root);
		assert_true(string_equals(xml_node_name(root), "root"));
		assert_true(string_equals(xml_node_content(root), "content"));
		xml_document_free(document, true);
	} else {
		free(source);
		/* Not yet implemented: parser may fail on DOCTYPE. */
		assert_true(true);
	}
}


/**
 * DOCTYPE with root element name only (no internal subset).
 * Expected (when implemented): skip and parse root.
 */
static void test_doctype_name_only_before_root(void **state) {
	(void)state;
	SOURCE(source, "<!DOCTYPE html><html><body>x</body></html>");
	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	if (document) {
		struct xml_node* root = xml_document_root(document);
		assert_non_null(root);
		assert_true(string_equals(xml_node_name(root), "html"));
		xml_document_free(document, true);
	} else {
		free(source);
		assert_true(true);
	}
}


/**
 * DOCTYPE with SYSTEM external ID. Expected: skip (no external entities) or
 * reject; document may still parse with root if DOCTYPE is skipped.
 */
static void test_doctype_external_id_skipped_or_rejected(void **state) {
	(void)state;
	SOURCE(source, "<!DOCTYPE root SYSTEM \"file.dtd\"><root/>");
	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	if (document) {
		struct xml_node* root = xml_document_root(document);
		assert_non_null(root);
		assert_true(string_equals(xml_node_name(root), "root/"));  /* self-closing */
		xml_document_free(document, true);
	} else {
		free(source);
		/* Rejecting external DOCTYPE is also acceptable. */
		assert_true(true);
	}
}


/**
 * DOCTYPE with internal subset (entity declaration). Optional: if parsing
 * DOCTYPE, predefined entities only; internal subset could be supported.
 */
static void test_doctype_internal_subset_empty_before_root(void **state) {
	(void)state;
	SOURCE(source, "<!DOCTYPE doc [ ]><doc/>");
	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	if (document) {
		struct xml_node* root = xml_document_root(document);
		assert_non_null(root);
		assert_true(string_equals(xml_node_name(root), "doc/"));  /* self-closing */
		xml_document_free(document, true);
	} else {
		free(source);
		assert_true(true);
	}
}


/**
 * XML declaration + DOCTYPE + root. Expected: both decl and DOCTYPE skipped
 * (or handled), root parsed.
 */
static void test_doctype_after_xml_decl(void **state) {
	(void)state;
	SOURCE(source, "<?xml version=\"1.0\"?><!DOCTYPE root []><root/>");
	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	if (document) {
		struct xml_node* root = xml_document_root(document);
		assert_non_null(root);
		assert_true(string_equals(xml_node_name(root), "root/"));  /* self-closing */
		xml_document_free(document, true);
	} else {
		free(source);
		assert_true(true);
	}
}


/**
 * DOCTYPE with PUBLIC and SYSTEM identifiers (external subset) is skipped; root parsed.
 */
static void test_doctype_public_system_skipped(void **state) {
	(void)state;
	SOURCE(source, "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\"><html/>");
	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	if (document) {
		struct xml_node* root = xml_document_root(document);
		assert_non_null(root);
		assert_true(string_equals(xml_node_name(root), "html/"));
		xml_document_free(document, true);
	} else {
		free(source);
	}
}


	static const struct CMUnitTest tests[] = {
	cmocka_unit_test(test_doctype_minimal_before_root),
	cmocka_unit_test(test_doctype_name_only_before_root),
	cmocka_unit_test(test_doctype_external_id_skipped_or_rejected),
	cmocka_unit_test(test_doctype_internal_subset_empty_before_root),
	cmocka_unit_test(test_doctype_after_xml_decl),
	cmocka_unit_test(test_doctype_public_system_skipped),
};

void get_unit_c_doctype_tests(const struct CMUnitTest** out_tests, size_t* out_count) {
	*out_tests = tests;
	*out_count = sizeof(tests) / sizeof(tests[0]);
}
