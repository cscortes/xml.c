/*
 * Feature unit tests for the _c_string helper API:
 *   xml_node_name_c_string           (NS-01..NS-05)
 *   xml_node_content_c_string        (CS-01..CS-06)
 *   xml_node_attribute_name_c_string (ANS-01..ANS-05)
 *   xml_node_attribute_content_c_string (ACS-01..ACS-06)
 * Derived from docs/FeatureTestCases.md.
 *
 * Returned pointers are null-terminated; caller must free().
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


/* --- xml_node_name_c_string ----------------------------------------------- */

/* NS-01: NULL node — returns NULL. */
static void test_ns01_name_cstr_null_node(void **state) {
	(void)state;
	assert_null(xml_node_name_c_string(NULL));
}

/* NS-02: Valid node — non-NULL; null-terminated; correct content. */
static void test_ns02_name_cstr_valid(void **state) {
	(void)state;
	SOURCE(s, "<hello></hello>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	uint8_t *cstr = xml_node_name_c_string(xml_document_root(doc));
	assert_non_null(cstr);
	assert_string_equal((char const *)cstr, "hello");
	free(cstr);
	xml_document_free(doc, true);
}

/* NS-03: Caller frees result — no crash. */
static void test_ns03_name_cstr_caller_frees(void **state) {
	(void)state;
	SOURCE(s, "<r/>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	uint8_t *cstr = xml_node_name_c_string(xml_document_root(doc));
	assert_non_null(cstr);
	free(cstr);
	xml_document_free(doc, true);
}

/* NS-04: Two calls return independent copies — modifying one does not affect the other. */
static void test_ns04_name_cstr_independent_copies(void **state) {
	(void)state;
	SOURCE(s, "<node></node>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	uint8_t *c1 = xml_node_name_c_string(xml_document_root(doc));
	uint8_t *c2 = xml_node_name_c_string(xml_document_root(doc));
	assert_non_null(c1);
	assert_non_null(c2);
	assert_ptr_not_equal(c1, c2);
	c1[0] = 'X';
	assert_string_equal((char const *)c2, "node");
	free(c1);
	free(c2);
	xml_document_free(doc, true);
}

/* NS-05: Prefixed name "ns:tag" — C string equals "ns:tag". */
static void test_ns05_name_cstr_prefixed(void **state) {
	(void)state;
	SOURCE(s, "<ns:tag></ns:tag>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	uint8_t *cstr = xml_node_name_c_string(xml_document_root(doc));
	assert_non_null(cstr);
	assert_string_equal((char const *)cstr, "ns:tag");
	free(cstr);
	xml_document_free(doc, true);
}


/* --- xml_node_content_c_string -------------------------------------------- */

/* CS-01: NULL node — returns NULL. */
static void test_cs01_content_cstr_null_node(void **state) {
	(void)state;
	assert_null(xml_node_content_c_string(NULL));
}

/* CS-02: Node with no content (self-closing) — returns NULL. */
static void test_cs02_content_cstr_self_closing(void **state) {
	(void)state;
	SOURCE(s, "<r/>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	assert_null(xml_node_content_c_string(xml_document_root(doc)));
	xml_document_free(doc, true);
}

/* CS-03: Node with content "hello" — non-NULL; equals "hello". */
static void test_cs03_content_cstr_valid(void **state) {
	(void)state;
	SOURCE(s, "<r>hello</r>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	uint8_t *cstr = xml_node_content_c_string(xml_document_root(doc));
	assert_non_null(cstr);
	assert_string_equal((char const *)cstr, "hello");
	free(cstr);
	xml_document_free(doc, true);
}

/* CS-04: Content with expanded entity <r>&amp;</r> — equals "&". */
static void test_cs04_content_cstr_entity(void **state) {
	(void)state;
	SOURCE(s, "<r>&amp;</r>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	uint8_t *cstr = xml_node_content_c_string(xml_document_root(doc));
	assert_non_null(cstr);
	assert_string_equal((char const *)cstr, "&");
	free(cstr);
	xml_document_free(doc, true);
}

/* CS-05: Caller frees result — no leak. */
static void test_cs05_content_cstr_caller_frees(void **state) {
	(void)state;
	SOURCE(s, "<r>data</r>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	uint8_t *cstr = xml_node_content_c_string(xml_document_root(doc));
	assert_non_null(cstr);
	free(cstr);
	xml_document_free(doc, true);
}

/* CS-06: Two calls return independent copies. */
static void test_cs06_content_cstr_independent_copies(void **state) {
	(void)state;
	SOURCE(s, "<r>text</r>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	uint8_t *c1 = xml_node_content_c_string(xml_document_root(doc));
	uint8_t *c2 = xml_node_content_c_string(xml_document_root(doc));
	assert_non_null(c1);
	assert_non_null(c2);
	assert_ptr_not_equal(c1, c2);
	free(c1);
	free(c2);
	xml_document_free(doc, true);
}


/* --- xml_node_attribute_name_c_string ------------------------------------- */

/* ANS-01: NULL node — returns NULL. */
static void test_ans01_attr_name_cstr_null(void **state) {
	(void)state;
	assert_null(xml_node_attribute_name_c_string(NULL, 0));
}

/* ANS-02: Out-of-range index — returns NULL. */
static void test_ans02_attr_name_cstr_out_of_range(void **state) {
	(void)state;
	SOURCE(s, "<r a=\"1\"/>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	assert_null(xml_node_attribute_name_c_string(xml_document_root(doc), 5));
	xml_document_free(doc, true);
}

/* ANS-03: Valid node + index — non-NULL; null-terminated; correct name. */
static void test_ans03_attr_name_cstr_valid(void **state) {
	(void)state;
	SOURCE(s, "<r myattr=\"v\"/>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	uint8_t *cstr = xml_node_attribute_name_c_string(xml_document_root(doc), 0);
	assert_non_null(cstr);
	assert_string_equal((char const *)cstr, "myattr");
	free(cstr);
	xml_document_free(doc, true);
}

/* ANS-04: Namespace attribute "xmlns:n" — C string equals "xmlns:n". */
static void test_ans04_xmlns_attr_name_cstr(void **state) {
	(void)state;
	SOURCE(s, "<r xmlns:n=\"http://x.com\"/>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	uint8_t *cstr = xml_node_attribute_name_c_string(xml_document_root(doc), 0);
	assert_non_null(cstr);
	assert_string_equal((char const *)cstr, "xmlns:n");
	free(cstr);
	xml_document_free(doc, true);
}

/* ANS-05: Caller frees result — no leak. */
static void test_ans05_attr_name_cstr_caller_frees(void **state) {
	(void)state;
	SOURCE(s, "<r k=\"v\"/>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	uint8_t *cstr = xml_node_attribute_name_c_string(xml_document_root(doc), 0);
	assert_non_null(cstr);
	free(cstr);
	xml_document_free(doc, true);
}


/* --- xml_node_attribute_content_c_string ---------------------------------- */

/* ACS-01: NULL node — returns NULL. */
static void test_acs01_attr_content_cstr_null(void **state) {
	(void)state;
	assert_null(xml_node_attribute_content_c_string(NULL, 0));
}

/* ACS-02: Out-of-range index — returns NULL. */
static void test_acs02_attr_content_cstr_out_of_range(void **state) {
	(void)state;
	SOURCE(s, "<r a=\"1\"/>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	assert_null(xml_node_attribute_content_c_string(xml_document_root(doc), 5));
	xml_document_free(doc, true);
}

/* ACS-03: Valid node + index — non-NULL; null-terminated; correct value. */
static void test_acs03_attr_content_cstr_valid(void **state) {
	(void)state;
	SOURCE(s, "<r k=\"world\"/>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	uint8_t *cstr = xml_node_attribute_content_c_string(xml_document_root(doc), 0);
	assert_non_null(cstr);
	assert_string_equal((char const *)cstr, "world");
	free(cstr);
	xml_document_free(doc, true);
}

/* ACS-04: Attribute with empty value — non-NULL; strlen == 0. */
static void test_acs04_attr_content_cstr_empty(void **state) {
	(void)state;
	SOURCE(s, "<r a=\"\"/>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	uint8_t *cstr = xml_node_attribute_content_c_string(xml_document_root(doc), 0);
	assert_non_null(cstr);
	assert_int_equal(strlen((char const *)cstr), 0);
	free(cstr);
	xml_document_free(doc, true);
}

/* ACS-05: Attribute with entity value &amp; — equals "&". */
static void test_acs05_attr_content_cstr_entity(void **state) {
	(void)state;
	SOURCE(s, "<r a=\"&amp;\"/>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	uint8_t *cstr = xml_node_attribute_content_c_string(xml_document_root(doc), 0);
	assert_non_null(cstr);
	assert_string_equal((char const *)cstr, "&");
	free(cstr);
	xml_document_free(doc, true);
}

/* ACS-06: Caller frees result — no leak. */
static void test_acs06_attr_content_cstr_caller_frees(void **state) {
	(void)state;
	SOURCE(s, "<r x=\"val\"/>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	uint8_t *cstr = xml_node_attribute_content_c_string(xml_document_root(doc), 0);
	assert_non_null(cstr);
	free(cstr);
	xml_document_free(doc, true);
}


static const struct CMUnitTest tests[] = {
	cmocka_unit_test(test_ns01_name_cstr_null_node),
	cmocka_unit_test(test_ns02_name_cstr_valid),
	cmocka_unit_test(test_ns03_name_cstr_caller_frees),
	cmocka_unit_test(test_ns04_name_cstr_independent_copies),
	cmocka_unit_test(test_ns05_name_cstr_prefixed),
	cmocka_unit_test(test_cs01_content_cstr_null_node),
	cmocka_unit_test(test_cs02_content_cstr_self_closing),
	cmocka_unit_test(test_cs03_content_cstr_valid),
	cmocka_unit_test(test_cs04_content_cstr_entity),
	cmocka_unit_test(test_cs05_content_cstr_caller_frees),
	cmocka_unit_test(test_cs06_content_cstr_independent_copies),
	cmocka_unit_test(test_ans01_attr_name_cstr_null),
	cmocka_unit_test(test_ans02_attr_name_cstr_out_of_range),
	cmocka_unit_test(test_ans03_attr_name_cstr_valid),
	cmocka_unit_test(test_ans04_xmlns_attr_name_cstr),
	cmocka_unit_test(test_ans05_attr_name_cstr_caller_frees),
	cmocka_unit_test(test_acs01_attr_content_cstr_null),
	cmocka_unit_test(test_acs02_attr_content_cstr_out_of_range),
	cmocka_unit_test(test_acs03_attr_content_cstr_valid),
	cmocka_unit_test(test_acs04_attr_content_cstr_empty),
	cmocka_unit_test(test_acs05_attr_content_cstr_entity),
	cmocka_unit_test(test_acs06_attr_content_cstr_caller_frees),
};

void get_ut_c_string_api_tests(const struct CMUnitTest **out_tests, size_t *out_count) {
	*out_tests = tests;
	*out_count = sizeof(tests) / sizeof(tests[0]);
}
