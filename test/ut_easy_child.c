/*
 * Feature unit tests for xml_easy_child — cases EC-01 through EC-11.
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


/* EC-01: NULL node — returns NULL. */
static void test_ec01_null_node(void **state) {
	(void)state;
	assert_null(xml_easy_child(NULL, (uint8_t const *)"Tag", (uint8_t const *)0));
}

/* EC-02: Single level, child exists — returns child node. */
static void test_ec02_single_level_exists(void **state) {
	(void)state;
	SOURCE(s, "<r><child></child></r>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	struct xml_node *child = xml_easy_child(xml_document_root(doc),
	                                        (uint8_t const *)"child",
	                                        (uint8_t const *)0);
	assert_non_null(child);
	assert_true(xml_str_eq(xml_node_name(child), "child"));
	xml_document_free(doc, true);
}

/* EC-03: Single level, child does not exist — returns NULL. */
static void test_ec03_single_level_missing(void **state) {
	(void)state;
	SOURCE(s, "<r><other/></r>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	assert_null(xml_easy_child(xml_document_root(doc),
	                           (uint8_t const *)"nothere",
	                           (uint8_t const *)0));
	xml_document_free(doc, true);
}

/* EC-04: Two-level path, both exist — returns grandchild. */
static void test_ec04_two_level_exists(void **state) {
	(void)state;
	SOURCE(s, "<r><a><b></b></a></r>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	struct xml_node *b = xml_easy_child(xml_document_root(doc),
	                                    (uint8_t const *)"a",
	                                    (uint8_t const *)"b",
	                                    (uint8_t const *)0);
	assert_non_null(b);
	assert_true(xml_str_eq(xml_node_name(b), "b"));
	xml_document_free(doc, true);
}

/* EC-05: Two-level path, intermediate missing — returns NULL. */
static void test_ec05_two_level_intermediate_missing(void **state) {
	(void)state;
	SOURCE(s, "<r><other><b/></other></r>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	assert_null(xml_easy_child(xml_document_root(doc),
	                           (uint8_t const *)"a",
	                           (uint8_t const *)"b",
	                           (uint8_t const *)0));
	xml_document_free(doc, true);
}

/* EC-06: Two-level path, final missing — returns NULL. */
static void test_ec06_two_level_final_missing(void **state) {
	(void)state;
	SOURCE(s, "<r><a/></r>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	assert_null(xml_easy_child(xml_document_root(doc),
	                           (uint8_t const *)"a",
	                           (uint8_t const *)"b",
	                           (uint8_t const *)0));
	xml_document_free(doc, true);
}

/* EC-07: Three-level path — returns correct node. */
static void test_ec07_three_level(void **state) {
	(void)state;
	SOURCE(s, "<r><a><b><c></c></b></a></r>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	struct xml_node *c = xml_easy_child(xml_document_root(doc),
	                                    (uint8_t const *)"a",
	                                    (uint8_t const *)"b",
	                                    (uint8_t const *)"c",
	                                    (uint8_t const *)0);
	assert_non_null(c);
	assert_true(xml_str_eq(xml_node_name(c), "c"));
	xml_document_free(doc, true);
}

/* EC-08: Duplicate child names at that level — ambiguous; returns NULL. */
static void test_ec08_duplicate_children_ambiguous(void **state) {
	(void)state;
	SOURCE(s, "<r><item/><item/></r>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	assert_null(xml_easy_child(xml_document_root(doc),
	                           (uint8_t const *)"item",
	                           (uint8_t const *)0));
	xml_document_free(doc, true);
}

/* EC-09: NULL sentinel as only argument (zero-step path) — returns the node itself. */
static void test_ec09_null_sentinel_only(void **state) {
	(void)state;
	SOURCE(s, "<r/>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	struct xml_node *root = xml_document_root(doc);
	struct xml_node *result = xml_easy_child(root, (uint8_t const *)0);
	assert_ptr_equal(result, root);
	xml_document_free(doc, true);
}

/* EC-10: Child name is an empty string — returns NULL. */
static void test_ec10_empty_child_name(void **state) {
	(void)state;
	SOURCE(s, "<r><c/></r>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	assert_null(xml_easy_child(xml_document_root(doc),
	                           (uint8_t const *)"",
	                           (uint8_t const *)0));
	xml_document_free(doc, true);
}

/* EC-11: Named child is unique with siblings of different names — returns it. */
static void test_ec11_unique_among_siblings(void **state) {
	(void)state;
	SOURCE(s, "<r><alpha></alpha><beta></beta><gamma></gamma></r>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	struct xml_node *beta = xml_easy_child(xml_document_root(doc),
	                                       (uint8_t const *)"beta",
	                                       (uint8_t const *)0);
	assert_non_null(beta);
	assert_true(xml_str_eq(xml_node_name(beta), "beta"));
	xml_document_free(doc, true);
}


static const struct CMUnitTest tests[] = {
	cmocka_unit_test(test_ec01_null_node),
	cmocka_unit_test(test_ec02_single_level_exists),
	cmocka_unit_test(test_ec03_single_level_missing),
	cmocka_unit_test(test_ec04_two_level_exists),
	cmocka_unit_test(test_ec05_two_level_intermediate_missing),
	cmocka_unit_test(test_ec06_two_level_final_missing),
	cmocka_unit_test(test_ec07_three_level),
	cmocka_unit_test(test_ec08_duplicate_children_ambiguous),
	cmocka_unit_test(test_ec09_null_sentinel_only),
	cmocka_unit_test(test_ec10_empty_child_name),
	cmocka_unit_test(test_ec11_unique_among_siblings),
};

void get_ut_easy_child_tests(const struct CMUnitTest **out_tests, size_t *out_count) {
	*out_tests = tests;
	*out_count = sizeof(tests) / sizeof(tests[0]);
}
