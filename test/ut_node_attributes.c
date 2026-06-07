/*
 * Feature unit tests for:
 *   xml_node_attributes      (AT-01..AT-06)
 *   xml_node_attribute_name  (AN-01..AN-06)
 *   xml_node_attribute_content (AC-01..AC-08)
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


/* --- xml_node_attributes -------------------------------------------------- */

/* AT-01: NULL node — returns 0. */
static void test_at01_attributes_null_node(void **state) {
	(void)state;
	assert_int_equal(xml_node_attributes(NULL), 0);
}

/* AT-02: Node with no attributes — returns 0. */
static void test_at02_no_attributes(void **state) {
	(void)state;
	SOURCE(s, "<r/>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	assert_int_equal(xml_node_attributes(xml_document_root(doc)), 0);
	xml_document_free(doc, true);
}

/* AT-03: Node with 1 attribute — returns 1. */
static void test_at03_one_attribute(void **state) {
	(void)state;
	SOURCE(s, "<r x=\"1\"/>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	assert_int_equal(xml_node_attributes(xml_document_root(doc)), 1);
	xml_document_free(doc, true);
}

/* AT-04: Node with 3 attributes — returns 3. */
static void test_at04_three_attributes(void **state) {
	(void)state;
	SOURCE(s, "<r a=\"1\" b=\"2\" c=\"3\"/>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	assert_int_equal(xml_node_attributes(xml_document_root(doc)), 3);
	xml_document_free(doc, true);
}

/* AT-05: Node with namespace attributes — xmlns counts as an attribute. */
static void test_at05_namespace_attribute_counted(void **state) {
	(void)state;
	SOURCE(s, "<r xmlns=\"http://example.com\" x=\"1\"/>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	assert_int_equal(xml_node_attributes(xml_document_root(doc)), 2);
	xml_document_free(doc, true);
}

/* AT-06: Node with 5 attributes — returns 5. */
static void test_at06_five_attributes(void **state) {
	(void)state;
	SOURCE(s, "<r a=\"1\" b=\"2\" c=\"3\" d=\"4\" e=\"5\"/>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	assert_int_equal(xml_node_attributes(xml_document_root(doc)), 5);
	xml_document_free(doc, true);
}


/* --- xml_node_attribute_name ---------------------------------------------- */

/* AN-01: NULL node — returns NULL. */
static void test_an01_attr_name_null_node(void **state) {
	(void)state;
	assert_null(xml_node_attribute_name(NULL, 0));
}

/* AN-02: Valid index 0 — correct name. */
static void test_an02_attr_name_valid(void **state) {
	(void)state;
	SOURCE(s, "<r key=\"val\"/>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	assert_true(xml_str_eq(xml_node_attribute_name(xml_document_root(doc), 0), "key"));
	xml_document_free(doc, true);
}

/* AN-03: Out-of-range index — returns NULL. */
static void test_an03_attr_name_out_of_range(void **state) {
	(void)state;
	SOURCE(s, "<r a=\"1\"/>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	assert_null(xml_node_attribute_name(xml_document_root(doc), 5));
	xml_document_free(doc, true);
}

/* AN-04: Namespace attribute name "xmlns:ns". */
static void test_an04_xmlns_prefix_name(void **state) {
	(void)state;
	SOURCE(s, "<r xmlns:ns=\"http://example.com\"/>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	assert_true(xml_str_eq(xml_node_attribute_name(xml_document_root(doc), 0), "xmlns:ns"));
	xml_document_free(doc, true);
}

/* AN-05: Attribute with colon in name — correct name string. */
static void test_an05_attr_name_with_colon(void **state) {
	(void)state;
	SOURCE(s, "<r xml:lang=\"en\"/>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	assert_true(xml_str_eq(xml_node_attribute_name(xml_document_root(doc), 0), "xml:lang"));
	xml_document_free(doc, true);
}

/* AN-06: Name length via xml_string_length — correct byte count. */
static void test_an06_attr_name_length(void **state) {
	(void)state;
	SOURCE(s, "<r name=\"v\"/>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	struct xml_string *name = xml_node_attribute_name(xml_document_root(doc), 0);
	assert_non_null(name);
	assert_int_equal(xml_string_length(name), 4); /* "name" = 4 bytes */
	xml_document_free(doc, true);
}


/* --- xml_node_attribute_content ------------------------------------------- */

/* AC-01: NULL node — returns NULL. */
static void test_ac01_attr_content_null_node(void **state) {
	(void)state;
	assert_null(xml_node_attribute_content(NULL, 0));
}

/* AC-02: Valid index 0 — correct value. */
static void test_ac02_attr_content_valid(void **state) {
	(void)state;
	SOURCE(s, "<r k=\"hello\"/>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	assert_true(xml_str_eq(xml_node_attribute_content(xml_document_root(doc), 0), "hello"));
	xml_document_free(doc, true);
}

/* AC-03: Out-of-range index — returns NULL. */
static void test_ac03_attr_content_out_of_range(void **state) {
	(void)state;
	SOURCE(s, "<r a=\"1\"/>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	assert_null(xml_node_attribute_content(xml_document_root(doc), 5));
	xml_document_free(doc, true);
}

/* AC-04: Attribute with empty value a="" — non-NULL; length = 0. */
static void test_ac04_empty_attr_value(void **state) {
	(void)state;
	SOURCE(s, "<r a=\"\"/>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	struct xml_string *val = xml_node_attribute_content(xml_document_root(doc), 0);
	assert_non_null(val);
	assert_int_equal(xml_string_length(val), 0);
	xml_document_free(doc, true);
}

/* AC-05: Attribute with spaces in value. */
static void test_ac05_attr_value_with_spaces(void **state) {
	(void)state;
	SOURCE(s, "<r a=\"hello world\"/>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	assert_true(xml_str_eq(xml_node_attribute_content(xml_document_root(doc), 0), "hello world"));
	xml_document_free(doc, true);
}

/* AC-06: Attribute with entity — expanded to "&". */
static void test_ac06_attr_entity_expanded(void **state) {
	(void)state;
	SOURCE(s, "<r a=\"&amp;\"/>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	assert_true(xml_str_eq(xml_node_attribute_content(xml_document_root(doc), 0), "&"));
	xml_document_free(doc, true);
}

/* AC-07: Attribute with decimal char ref — expanded to "A". */
static void test_ac07_attr_char_ref_decimal(void **state) {
	(void)state;
	SOURCE(s, "<r a=\"&#65;\"/>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	assert_true(xml_str_eq(xml_node_attribute_content(xml_document_root(doc), 0), "A"));
	xml_document_free(doc, true);
}

/* AC-08: Attribute with single-quote delimiter — value equals "val". */
static void test_ac08_single_quote_delimiter(void **state) {
	(void)state;
	SOURCE(s, "<r a='val'/>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	assert_true(xml_str_eq(xml_node_attribute_content(xml_document_root(doc), 0), "val"));
	xml_document_free(doc, true);
}


static const struct CMUnitTest tests[] = {
	cmocka_unit_test(test_at01_attributes_null_node),
	cmocka_unit_test(test_at02_no_attributes),
	cmocka_unit_test(test_at03_one_attribute),
	cmocka_unit_test(test_at04_three_attributes),
	cmocka_unit_test(test_at05_namespace_attribute_counted),
	cmocka_unit_test(test_at06_five_attributes),
	cmocka_unit_test(test_an01_attr_name_null_node),
	cmocka_unit_test(test_an02_attr_name_valid),
	cmocka_unit_test(test_an03_attr_name_out_of_range),
	cmocka_unit_test(test_an04_xmlns_prefix_name),
	cmocka_unit_test(test_an05_attr_name_with_colon),
	cmocka_unit_test(test_an06_attr_name_length),
	cmocka_unit_test(test_ac01_attr_content_null_node),
	cmocka_unit_test(test_ac02_attr_content_valid),
	cmocka_unit_test(test_ac03_attr_content_out_of_range),
	cmocka_unit_test(test_ac04_empty_attr_value),
	cmocka_unit_test(test_ac05_attr_value_with_spaces),
	cmocka_unit_test(test_ac06_attr_entity_expanded),
	cmocka_unit_test(test_ac07_attr_char_ref_decimal),
	cmocka_unit_test(test_ac08_single_quote_delimiter),
};

void get_ut_node_attributes_tests(const struct CMUnitTest **out_tests, size_t *out_count) {
	*out_tests = tests;
	*out_count = sizeof(tests) / sizeof(tests[0]);
}
