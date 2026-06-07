/*
 * Feature unit tests for xml_document_root (R-01..R-04) and
 * xml_document_buffer_length (BL-01..BL-03).
 * Derived from docs/FeatureTestCases.md.
 *
 * Run with:  ctest -L unit
 */
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <cmocka.h>
#include <xml.h>
#include "ut_helpers.h"
#include "ut_runner.h"


/* R-01: xml_document_root(NULL) — returns NULL. */
static void test_r01_root_null_document(void **state) {
	(void)state;
	assert_null(xml_document_root(NULL));
}

/* R-02: Valid document <root/> — root node is non-NULL with name "root". */
static void test_r02_root_name(void **state) {
	(void)state;
	SOURCE(s, "<root></root>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	struct xml_node *root = xml_document_root(doc);
	assert_non_null(root);
	assert_true(xml_str_eq(xml_node_name(root), "root"));
	xml_document_free(doc, true);
}

/* R-03: Namespaced root <ns:root/> — name is "ns:root". */
static void test_r03_namespaced_root(void **state) {
	(void)state;
	SOURCE(s, "<ns:root></ns:root>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	struct xml_node *root = xml_document_root(doc);
	assert_non_null(root);
	assert_true(xml_str_eq(xml_node_name(root), "ns:root"));
	xml_document_free(doc, true);
}

/* R-04: Root name compared with xml_string_equals_cstr — matches. */
static void test_r04_root_name_equals_cstr(void **state) {
	(void)state;
	SOURCE(s, "<myroot></myroot>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	struct xml_node *root = xml_document_root(doc);
	assert_true(xml_string_equals_cstr(xml_node_name(root), (uint8_t const *)"myroot"));
	assert_false(xml_string_equals_cstr(xml_node_name(root), (uint8_t const *)"other"));
	xml_document_free(doc, true);
}


/* BL-01: xml_document_buffer_length(NULL) — returns 0. */
static void test_bl01_buffer_length_null(void **state) {
	(void)state;
	assert_int_equal(xml_document_buffer_length(NULL), 0);
}

/* BL-02: Document parsed from a 7-byte buffer — returns 7. */
static void test_bl02_buffer_length_known(void **state) {
	(void)state;
	/* "<r/>" is 4 bytes; "<r></r>" is 7 bytes */
	SOURCE(s, "<r></r>");
	size_t len = strlen((char const *)s);
	struct xml_document *doc = xml_parse_document(s, len);
	assert_non_null(doc);
	assert_int_equal(xml_document_buffer_length(doc), 7);
	xml_document_free(doc, true);
}

/* BL-03: Document opened from file — returns file size in bytes. */
static void test_bl03_buffer_length_from_file(void **state) {
	(void)state;
	char const *content = "<doc><item/></doc>";
	size_t expected = strlen(content);
	FILE *f = tmpfile();
	assert_non_null(f);
	fwrite(content, 1, expected, f);
	rewind(f);
	struct xml_document *doc = xml_open_document(f);
	assert_non_null(doc);
	assert_int_equal(xml_document_buffer_length(doc), expected);
	xml_document_free(doc, true);
}


static const struct CMUnitTest tests[] = {
	cmocka_unit_test(test_r01_root_null_document),
	cmocka_unit_test(test_r02_root_name),
	cmocka_unit_test(test_r03_namespaced_root),
	cmocka_unit_test(test_r04_root_name_equals_cstr),
	cmocka_unit_test(test_bl01_buffer_length_null),
	cmocka_unit_test(test_bl02_buffer_length_known),
	cmocka_unit_test(test_bl03_buffer_length_from_file),
};

void get_ut_document_root_tests(const struct CMUnitTest **out_tests, size_t *out_count) {
	*out_tests = tests;
	*out_count = sizeof(tests) / sizeof(tests[0]);
}
