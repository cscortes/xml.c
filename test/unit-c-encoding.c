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
 * Tests for encoding declaration (docs/issues.md):
 * Honor encoding in <?xml ...?> for rejection of unsupported encodings.
 * UTF-8 is the only assumed/supported encoding.
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
 * UTF-8 encoding in XML declaration is accepted.
 */
static void test_encoding_utf8_accepted(void **state) {
	(void)state;
	SOURCE(source, "<?xml encoding=\"UTF-8\"?><r>data</r>");
	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	assert_non_null(document);
	if (document) {
		struct xml_node* root = xml_document_root(document);
		assert_non_null(root);
		assert_true(string_equals(xml_node_name(root), "r"));
		assert_true(string_equals(xml_node_content(root), "data"));
		xml_document_free(document, true);
	} else {
		free(source);
	}
}


/**
 * UTF-8 encoding with lowercase/case variation is accepted (XML is case-insensitive for encoding).
 */
static void test_encoding_utf8_case_insensitive(void **state) {
	(void)state;
	SOURCE(source, "<?xml encoding=\"utf-8\"?><r/>");
	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	assert_non_null(document);
	if (document) {
		struct xml_node* root = xml_document_root(document);
		assert_non_null(root);
		assert_true(string_equals(xml_node_name(root), "r/"));  /* self-closing */
		xml_document_free(document, true);
	} else {
		free(source);
	}
}


/**
 * Unsupported encoding (e.g. ISO-8859-1) must be rejected.
 * Parser does not accept documents that declare a non-UTF-8 encoding.
 */
static void test_encoding_unsupported_rejected(void **state) {
	(void)state;
	SOURCE(source, "<?xml encoding=\"ISO-8859-1\"?><r/>");
	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	if (document) {
		xml_document_free(document, true);
		fail_msg("%s", "Expected parse failure for unsupported encoding ISO-8859-1");
	}
	free(source);
	assert_null(document);
}


/**
 * Another unsupported encoding (Windows-1252) must be rejected.
 */
static void test_encoding_unsupported_win1252_rejected(void **state) {
	(void)state;
	SOURCE(source, "<?xml encoding=\"Windows-1252\"?><r/>");
	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	if (document) {
		xml_document_free(document, true);
		fail_msg("%s", "Expected parse failure for unsupported encoding Windows-1252");
	}
	free(source);
	assert_null(document);
}


/**
 * No encoding in declaration: default is UTF-8, document should parse.
 */
static void test_encoding_absent_default_utf8(void **state) {
	(void)state;
	SOURCE(source, "<?xml version=\"1.0\"?><r/>");
	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	assert_non_null(document);
	if (document) {
		struct xml_node* root = xml_document_root(document);
		assert_non_null(root);
		xml_document_free(document, true);
	} else {
		free(source);
	}
}


static const struct CMUnitTest tests[] = {
	cmocka_unit_test(test_encoding_utf8_accepted),
	cmocka_unit_test(test_encoding_utf8_case_insensitive),
	cmocka_unit_test(test_encoding_unsupported_rejected),
	cmocka_unit_test(test_encoding_unsupported_win1252_rejected),
	cmocka_unit_test(test_encoding_absent_default_utf8),
};

void get_unit_c_encoding_tests(const struct CMUnitTest** out_tests, size_t* out_count) {
	*out_tests = tests;
	*out_count = sizeof(tests) / sizeof(tests[0]);
}
