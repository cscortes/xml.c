/*
 * Feature unit tests for the xml_string API:
 *   xml_string_length      (SL-01..SL-06)
 *   xml_string_copy        (SC-01..SC-06)
 *   xml_string_equals      (SE-01..SE-08, skip SE-06 impossible by construction)
 *   xml_string_equals_cstr (SEC-01..SEC-09)
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


/* --- xml_string_length ---------------------------------------------------- */

/* SL-01: NULL string — returns 0. */
static void test_sl01_length_null(void **state) {
	(void)state;
	assert_int_equal(xml_string_length(NULL), 0);
}

/* SL-02: Node name "root" — length is 4. */
static void test_sl02_length_name(void **state) {
	(void)state;
	SOURCE(s, "<root></root>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	assert_int_equal(xml_string_length(xml_node_name(xml_document_root(doc))), 4);
	xml_document_free(doc, true);
}

/* SL-03: Node content "hello world" — length is 11. */
static void test_sl03_length_content(void **state) {
	(void)state;
	SOURCE(s, "<r>hello world</r>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	assert_int_equal(xml_string_length(xml_node_content(xml_document_root(doc))), 11);
	xml_document_free(doc, true);
}

/* SL-04: Empty content <r></r> — content NULL (length not applicable); verified separately. */
static void test_sl04_length_empty_content(void **state) {
	(void)state;
	SOURCE(s, "<r></r>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	/* Empty element has NULL content; xml_string_length(NULL) == 0. */
	assert_int_equal(xml_string_length(xml_node_content(xml_document_root(doc))), 0);
	xml_document_free(doc, true);
}

/* SL-05: Expanded entity &amp; → "&" — length is 1 (one byte). */
static void test_sl05_length_entity_expanded(void **state) {
	(void)state;
	SOURCE(s, "<r>&amp;</r>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	assert_int_equal(xml_string_length(xml_node_content(xml_document_root(doc))), 1);
	xml_document_free(doc, true);
}

/* SL-06: Multi-byte UTF-8 character "é" (U+00E9 = 0xC3 0xA9) — length is 2 bytes. */
static void test_sl06_length_multibyte_utf8(void **state) {
	(void)state;
	/* "é" in UTF-8 is 2 bytes: 0xC3 0xA9 */
	SOURCE(s, "<r>\xC3\xA9</r>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	assert_int_equal(xml_string_length(xml_node_content(xml_document_root(doc))), 2);
	xml_document_free(doc, true);
}


/* --- xml_string_copy ------------------------------------------------------ */

/* SC-01: Buffer exactly the string length — all bytes written; no null terminator. */
static void test_sc01_copy_exact_buffer(void **state) {
	(void)state;
	SOURCE(s, "<r>hi</r>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	struct xml_string *content = xml_node_content(xml_document_root(doc));
	assert_non_null(content);
	size_t len = xml_string_length(content); /* 2 */
	uint8_t buf[2];
	memset(buf, 0xFF, sizeof(buf));
	xml_string_copy(content, buf, len);
	assert_int_equal(buf[0], 'h');
	assert_int_equal(buf[1], 'i');
	xml_document_free(doc, true);
}

/* SC-02: Buffer larger than string — correct bytes written; sentinel beyond end is untouched. */
static void test_sc02_copy_larger_buffer(void **state) {
	(void)state;
	SOURCE(s, "<r>hi</r>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	struct xml_string *content = xml_node_content(xml_document_root(doc));
	assert_non_null(content);
	uint8_t buf[8];
	memset(buf, 0xAB, sizeof(buf));
	xml_string_copy(content, buf, sizeof(buf));
	assert_int_equal(buf[0], 'h');
	assert_int_equal(buf[1], 'i');
	/* Bytes beyond the string are untouched. */
	assert_int_equal(buf[2], 0xAB);
	xml_document_free(doc, true);
}

/* SC-03: Buffer smaller than string — at most length bytes written; no overflow. */
static void test_sc03_copy_smaller_buffer(void **state) {
	(void)state;
	SOURCE(s, "<r>hello</r>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	struct xml_string *content = xml_node_content(xml_document_root(doc));
	assert_non_null(content);
	uint8_t buf[3];
	uint8_t sentinel = 0xBB;
	/* Place sentinel immediately after the 3-byte buffer to detect overflow. */
	uint8_t guard = sentinel;
	xml_string_copy(content, buf, 3);
	assert_int_equal(buf[0], 'h');
	assert_int_equal(buf[1], 'e');
	assert_int_equal(buf[2], 'l');
	assert_int_equal(guard, sentinel); /* stack sentinel unchanged */
	xml_document_free(doc, true);
}

/* SC-04: length=0 — nothing written; no crash. */
static void test_sc04_copy_zero_length(void **state) {
	(void)state;
	SOURCE(s, "<r>data</r>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	struct xml_string *content = xml_node_content(xml_document_root(doc));
	uint8_t buf[4] = { 0xAA, 0xAA, 0xAA, 0xAA };
	xml_string_copy(content, buf, 0);
	assert_int_equal(buf[0], 0xAA); /* unchanged */
	xml_document_free(doc, true);
}

/* SC-05: NULL string — no crash; nothing written. */
static void test_sc05_copy_null_string(void **state) {
	(void)state;
	uint8_t buf[4] = { 0xCC, 0xCC, 0xCC, 0xCC };
	xml_string_copy(NULL, buf, sizeof(buf));
	/* No crash; buffer may or may not be modified (implementation-defined). */
	(void)buf;
}

/* SC-06: Copy then compare with memcmp — copied bytes match source. */
static void test_sc06_copy_then_compare(void **state) {
	(void)state;
	SOURCE(s, "<r>abc</r>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	struct xml_string *content = xml_node_content(xml_document_root(doc));
	assert_non_null(content);
	size_t len = xml_string_length(content);
	uint8_t *buf = malloc(len);
	assert_non_null(buf);
	xml_string_copy(content, buf, len);
	assert_memory_equal(buf, "abc", len);
	free(buf);
	xml_document_free(doc, true);
}


/* --- xml_string_equals ---------------------------------------------------- */

/* SE-01: Both NULL — false. */
static void test_se01_equals_both_null(void **state) {
	(void)state;
	assert_false(xml_string_equals(NULL, NULL));
}

/* SE-02: First NULL, second valid — false. */
static void test_se02_equals_first_null(void **state) {
	(void)state;
	SOURCE(s, "<r/>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	assert_false(xml_string_equals(NULL, xml_node_name(xml_document_root(doc))));
	xml_document_free(doc, true);
}

/* SE-03: First valid, second NULL — false. */
static void test_se03_equals_second_null(void **state) {
	(void)state;
	SOURCE(s, "<r/>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	assert_false(xml_string_equals(xml_node_name(xml_document_root(doc)), NULL));
	xml_document_free(doc, true);
}

/* SE-04: Two nodes with same name — true. */
static void test_se04_equals_same_name(void **state) {
	(void)state;
	SOURCE(s, "<r><item/><item/></r>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	struct xml_node *root = xml_document_root(doc);
	struct xml_string *n1 = xml_node_name(xml_node_child(root, 0));
	struct xml_string *n2 = xml_node_name(xml_node_child(root, 1));
	assert_true(xml_string_equals(n1, n2));
	xml_document_free(doc, true);
}

/* SE-05: Two nodes with different names — false. */
static void test_se05_equals_different_names(void **state) {
	(void)state;
	SOURCE(s, "<r><a/><b/></r>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	struct xml_node *root = xml_document_root(doc);
	struct xml_string *n1 = xml_node_name(xml_node_child(root, 0));
	struct xml_string *n2 = xml_node_name(xml_node_child(root, 1));
	assert_false(xml_string_equals(n1, n2));
	xml_document_free(doc, true);
}

/* SE-06: (Skipped) Same content, different lengths — impossible by construction. */

/* SE-07: Same bytes, same length — true. */
static void test_se07_equals_same_bytes(void **state) {
	(void)state;
	SOURCE(s, "<r><x>foo</x><x>foo</x></r>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	struct xml_node *root = xml_document_root(doc);
	struct xml_string *c1 = xml_node_content(xml_node_child(root, 0));
	struct xml_string *c2 = xml_node_content(xml_node_child(root, 1));
	assert_true(xml_string_equals(c1, c2));
	xml_document_free(doc, true);
}

/* SE-08: Case-different strings "Foo" vs "foo" — false (case-sensitive). */
static void test_se08_equals_case_sensitive(void **state) {
	(void)state;
	SOURCE(s, "<r><Foo/><foo/></r>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	struct xml_node *root = xml_document_root(doc);
	struct xml_string *n1 = xml_node_name(xml_node_child(root, 0));
	struct xml_string *n2 = xml_node_name(xml_node_child(root, 1));
	assert_false(xml_string_equals(n1, n2));
	xml_document_free(doc, true);
}


/* --- xml_string_equals_cstr ----------------------------------------------- */

/* SEC-01: NULL string, non-NULL cstr — false. */
static void test_sec01_ecstr_null_string(void **state) {
	(void)state;
	assert_false(xml_string_equals_cstr(NULL, (uint8_t const *)"x"));
}

/* SEC-02: Valid non-empty string, NULL cstr — false (NULL treated as empty). */
static void test_sec02_ecstr_null_cstr_nonempty(void **state) {
	(void)state;
	SOURCE(s, "<r>hello</r>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	assert_false(xml_string_equals_cstr(xml_node_content(xml_document_root(doc)), NULL));
	xml_document_free(doc, true);
}

/* SEC-03: Valid string, matching cstr — true. */
static void test_sec03_ecstr_match(void **state) {
	(void)state;
	SOURCE(s, "<root></root>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	assert_true(xml_string_equals_cstr(xml_node_name(xml_document_root(doc)),
	                                   (uint8_t const *)"root"));
	xml_document_free(doc, true);
}

/* SEC-04: Valid string, non-matching cstr — false. */
static void test_sec04_ecstr_no_match(void **state) {
	(void)state;
	SOURCE(s, "<root/>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	assert_false(xml_string_equals_cstr(xml_node_name(xml_document_root(doc)),
	                                    (uint8_t const *)"other"));
	xml_document_free(doc, true);
}

/* SEC-05: Valid string, cstr with extra trailing char — false. */
static void test_sec05_ecstr_extra_char(void **state) {
	(void)state;
	SOURCE(s, "<root/>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	assert_false(xml_string_equals_cstr(xml_node_name(xml_document_root(doc)),
	                                    (uint8_t const *)"rootX"));
	xml_document_free(doc, true);
}

/* SEC-06: Case-different "Root" vs "root" — false (case-sensitive). */
static void test_sec06_ecstr_case_sensitive(void **state) {
	(void)state;
	SOURCE(s, "<Root/>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	assert_false(xml_string_equals_cstr(xml_node_name(xml_document_root(doc)),
	                                    (uint8_t const *)"root"));
	xml_document_free(doc, true);
}

/* SEC-07: Empty xml_string vs empty cstr "" — true. */
static void test_sec07_ecstr_empty_vs_empty(void **state) {
	(void)state;
	SOURCE(s, "<r a=\"\"/>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	struct xml_string *val = xml_node_attribute_content(xml_document_root(doc), 0);
	assert_non_null(val);
	assert_true(xml_string_equals_cstr(val, (uint8_t const *)""));
	xml_document_free(doc, true);
}

/* SEC-08: Empty xml_string vs NULL cstr — true (NULL treated as empty). */
static void test_sec08_ecstr_empty_vs_null(void **state) {
	(void)state;
	SOURCE(s, "<r a=\"\"/>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	struct xml_string *val = xml_node_attribute_content(xml_document_root(doc), 0);
	assert_non_null(val);
	assert_true(xml_string_equals_cstr(val, NULL));
	xml_document_free(doc, true);
}

/* SEC-09: Prefixed name "ns:tag" vs "ns:tag" — true. */
static void test_sec09_ecstr_prefixed_name(void **state) {
	(void)state;
	SOURCE(s, "<ns:tag></ns:tag>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	assert_true(xml_string_equals_cstr(xml_node_name(xml_document_root(doc)),
	                                   (uint8_t const *)"ns:tag"));
	xml_document_free(doc, true);
}


static const struct CMUnitTest tests[] = {
	cmocka_unit_test(test_sl01_length_null),
	cmocka_unit_test(test_sl02_length_name),
	cmocka_unit_test(test_sl03_length_content),
	cmocka_unit_test(test_sl04_length_empty_content),
	cmocka_unit_test(test_sl05_length_entity_expanded),
	cmocka_unit_test(test_sl06_length_multibyte_utf8),
	cmocka_unit_test(test_sc01_copy_exact_buffer),
	cmocka_unit_test(test_sc02_copy_larger_buffer),
	cmocka_unit_test(test_sc03_copy_smaller_buffer),
	cmocka_unit_test(test_sc04_copy_zero_length),
	cmocka_unit_test(test_sc05_copy_null_string),
	cmocka_unit_test(test_sc06_copy_then_compare),
	cmocka_unit_test(test_se01_equals_both_null),
	cmocka_unit_test(test_se02_equals_first_null),
	cmocka_unit_test(test_se03_equals_second_null),
	cmocka_unit_test(test_se04_equals_same_name),
	cmocka_unit_test(test_se05_equals_different_names),
	cmocka_unit_test(test_se07_equals_same_bytes),
	cmocka_unit_test(test_se08_equals_case_sensitive),
	cmocka_unit_test(test_sec01_ecstr_null_string),
	cmocka_unit_test(test_sec02_ecstr_null_cstr_nonempty),
	cmocka_unit_test(test_sec03_ecstr_match),
	cmocka_unit_test(test_sec04_ecstr_no_match),
	cmocka_unit_test(test_sec05_ecstr_extra_char),
	cmocka_unit_test(test_sec06_ecstr_case_sensitive),
	cmocka_unit_test(test_sec07_ecstr_empty_vs_empty),
	cmocka_unit_test(test_sec08_ecstr_empty_vs_null),
	cmocka_unit_test(test_sec09_ecstr_prefixed_name),
};

void get_ut_string_api_tests(const struct CMUnitTest **out_tests, size_t *out_count) {
	*out_tests = tests;
	*out_count = sizeof(tests) / sizeof(tests[0]);
}
