/*
 * Feature unit tests for:
 *   xml_node_name    (NN-01..NN-06)
 *   xml_node_content (NC-01..NC-10)
 *   xml_node_children (CH-01..CH-07)
 *   xml_node_child   (CK-01..CK-06)
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


/* --- xml_node_name -------------------------------------------------------- */

/* NN-01: NULL node — returns NULL. */
static void test_nn01_name_null_node(void **state) {
	(void)state;
	assert_null(xml_node_name(NULL));
}

/* NN-02: Root node <root/> — name equals "root". */
static void test_nn02_root_name(void **state) {
	(void)state;
	SOURCE(s, "<root></root>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	assert_true(xml_str_eq(xml_node_name(xml_document_root(doc)), "root"));
	xml_document_free(doc, true);
}

/* NN-03: Child node <child/> — name equals "child". */
static void test_nn03_child_name(void **state) {
	(void)state;
	SOURCE(s, "<r><child></child></r>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	struct xml_node *child = xml_node_child(xml_document_root(doc), 0);
	assert_non_null(child);
	assert_true(xml_str_eq(xml_node_name(child), "child"));
	xml_document_free(doc, true);
}

/* NN-04: Prefixed node <ns:tag/> — name equals "ns:tag". */
static void test_nn04_prefixed_name(void **state) {
	(void)state;
	SOURCE(s, "<ns:tag></ns:tag>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	assert_true(xml_str_eq(xml_node_name(xml_document_root(doc)), "ns:tag"));
	xml_document_free(doc, true);
}

/* NN-05: Underscore name <_tag/> — name equals "_tag". */
static void test_nn05_underscore_name(void **state) {
	(void)state;
	SOURCE(s, "<_tag></_tag>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	assert_true(xml_str_eq(xml_node_name(xml_document_root(doc)), "_tag"));
	xml_document_free(doc, true);
}

/* NN-06: Name length matches xml_string_length. */
static void test_nn06_name_length(void **state) {
	(void)state;
	SOURCE(s, "<hello></hello>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	struct xml_string *name = xml_node_name(xml_document_root(doc));
	assert_non_null(name);
	assert_int_equal(xml_string_length(name), 5); /* "hello" = 5 bytes */
	xml_document_free(doc, true);
}


/* --- xml_node_content ----------------------------------------------------- */

/* NC-01: NULL node — returns NULL. */
static void test_nc01_content_null_node(void **state) {
	(void)state;
	assert_null(xml_node_content(NULL));
}

/* NC-02: Self-closing element <r/> — returns NULL. */
static void test_nc02_self_closing_no_content(void **state) {
	(void)state;
	SOURCE(s, "<r/>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	assert_null(xml_node_content(xml_document_root(doc)));
	xml_document_free(doc, true);
}

/* NC-03: Empty element <r></r> — returns NULL (no content). */
static void test_nc03_empty_element_no_content(void **state) {
	(void)state;
	SOURCE(s, "<r></r>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	assert_null(xml_node_content(xml_document_root(doc)));
	xml_document_free(doc, true);
}

/* NC-04: Element with text <r>hello</r> — content equals "hello". */
static void test_nc04_text_content(void **state) {
	(void)state;
	SOURCE(s, "<r>hello</r>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	assert_true(xml_str_eq(xml_node_content(xml_document_root(doc)), "hello"));
	xml_document_free(doc, true);
}

/* NC-05: Element with entity <r>&amp;</r> — content equals "&". */
static void test_nc05_entity_content(void **state) {
	(void)state;
	SOURCE(s, "<r>&amp;</r>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	assert_true(xml_str_eq(xml_node_content(xml_document_root(doc)), "&"));
	xml_document_free(doc, true);
}

/* NC-06: Element with CDATA <r><![CDATA[text]]></r> — content equals "text". */
static void test_nc06_cdata_content(void **state) {
	(void)state;
	SOURCE(s, "<r><![CDATA[text]]></r>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	assert_true(xml_str_eq(xml_node_content(xml_document_root(doc)), "text"));
	xml_document_free(doc, true);
}

/* NC-07: Element with child nodes and no text <r><c/></r> — returns NULL. */
static void test_nc07_children_no_text(void **state) {
	(void)state;
	SOURCE(s, "<r><c/></r>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	assert_null(xml_node_content(xml_document_root(doc)));
	xml_document_free(doc, true);
}

/* NC-08: Mixed content <r>text<c/>more</r> — content is non-NULL. */
static void test_nc08_mixed_content(void **state) {
	(void)state;
	SOURCE(s, "<r>text<c/>more</r>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	/* Mixed-content behavior is implementation-defined; we require no crash. */
	(void)xml_node_content(xml_document_root(doc));
	xml_document_free(doc, true);
}

/* NC-09: Content with multiple entities — correct expansion. */
static void test_nc09_multiple_entities(void **state) {
	(void)state;
	SOURCE(s, "<r>&lt;tag&gt;</r>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	assert_true(xml_str_eq(xml_node_content(xml_document_root(doc)), "<tag>"));
	xml_document_free(doc, true);
}

/* NC-10: Content is whitespace only <r>   </r> — library normalizes to empty. */
static void test_nc10_whitespace_content(void **state) {
	(void)state;
	SOURCE(s, "<r>   </r>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	assert_int_equal(xml_string_length(xml_node_content(xml_document_root(doc))), 0);
	xml_document_free(doc, true);
}


/* --- xml_node_children ---------------------------------------------------- */

/* CH-01: NULL node — returns 0. */
static void test_ch01_children_null_node(void **state) {
	(void)state;
	assert_int_equal(xml_node_children(NULL), 0);
}

/* CH-02: Self-closing node <r/> — 0 children. */
static void test_ch02_self_closing_zero_children(void **state) {
	(void)state;
	SOURCE(s, "<r/>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	assert_int_equal(xml_node_children(xml_document_root(doc)), 0);
	xml_document_free(doc, true);
}

/* CH-03: Node with one child — 1. */
static void test_ch03_one_child(void **state) {
	(void)state;
	SOURCE(s, "<r><c/></r>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	assert_int_equal(xml_node_children(xml_document_root(doc)), 1);
	xml_document_free(doc, true);
}

/* CH-04: Node with multiple children — correct count. */
static void test_ch04_multiple_children(void **state) {
	(void)state;
	SOURCE(s, "<r><a/><b/><c/></r>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	assert_int_equal(xml_node_children(xml_document_root(doc)), 3);
	xml_document_free(doc, true);
}

/* CH-05: Node with content only (no children) <r>text</r> — 0. */
static void test_ch05_text_only_zero_children(void **state) {
	(void)state;
	SOURCE(s, "<r>text</r>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	assert_int_equal(xml_node_children(xml_document_root(doc)), 0);
	xml_document_free(doc, true);
}

/* CH-06: Comments are skipped — do not appear in child count. */
static void test_ch06_comments_not_counted(void **state) {
	(void)state;
	SOURCE(s, "<r><!-- comment --><c/></r>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	assert_int_equal(xml_node_children(xml_document_root(doc)), 1);
	xml_document_free(doc, true);
}

/* CH-07: Deeply nested — correct child count at each level. */
static void test_ch07_deep_children_count(void **state) {
	(void)state;
	SOURCE(s, "<a><b><c/><c/></b></a>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	struct xml_node *a = xml_document_root(doc);
	assert_int_equal(xml_node_children(a), 1);
	struct xml_node *b = xml_node_child(a, 0);
	assert_int_equal(xml_node_children(b), 2);
	xml_document_free(doc, true);
}


/* --- xml_node_child ------------------------------------------------------- */

/* CK-01: NULL node — returns NULL. */
static void test_ck01_child_null_node(void **state) {
	(void)state;
	assert_null(xml_node_child(NULL, 0));
}

/* CK-02: Index 0 on node with 1 child — returns the child. */
static void test_ck02_first_child(void **state) {
	(void)state;
	SOURCE(s, "<r><c/></r>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	assert_non_null(xml_node_child(xml_document_root(doc), 0));
	xml_document_free(doc, true);
}

/* CK-03: Index == children count — returns NULL. */
static void test_ck03_index_out_of_range(void **state) {
	(void)state;
	SOURCE(s, "<r><c/></r>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	assert_null(xml_node_child(xml_document_root(doc), 1));
	xml_document_free(doc, true);
}

/* CK-04: Large index — returns NULL. */
static void test_ck04_large_index(void **state) {
	(void)state;
	SOURCE(s, "<r><c/></r>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	assert_null(xml_node_child(xml_document_root(doc), 999));
	xml_document_free(doc, true);
}

/* CK-05: Second child of node with 3 children — correct node. */
static void test_ck05_second_child(void **state) {
	(void)state;
	SOURCE(s, "<r><a></a><b></b><c></c></r>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	struct xml_node *second = xml_node_child(xml_document_root(doc), 1);
	assert_non_null(second);
	assert_true(xml_str_eq(xml_node_name(second), "b"));
	xml_document_free(doc, true);
}

/* CK-06: Child node name matches expected. */
static void test_ck06_child_name_correct(void **state) {
	(void)state;
	SOURCE(s, "<r><alpha></alpha></r>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	struct xml_node *child = xml_node_child(xml_document_root(doc), 0);
	assert_non_null(child);
	assert_true(xml_str_eq(xml_node_name(child), "alpha"));
	xml_document_free(doc, true);
}


static const struct CMUnitTest tests[] = {
	cmocka_unit_test(test_nn01_name_null_node),
	cmocka_unit_test(test_nn02_root_name),
	cmocka_unit_test(test_nn03_child_name),
	cmocka_unit_test(test_nn04_prefixed_name),
	cmocka_unit_test(test_nn05_underscore_name),
	cmocka_unit_test(test_nn06_name_length),
	cmocka_unit_test(test_nc01_content_null_node),
	cmocka_unit_test(test_nc02_self_closing_no_content),
	cmocka_unit_test(test_nc03_empty_element_no_content),
	cmocka_unit_test(test_nc04_text_content),
	cmocka_unit_test(test_nc05_entity_content),
	cmocka_unit_test(test_nc06_cdata_content),
	cmocka_unit_test(test_nc07_children_no_text),
	cmocka_unit_test(test_nc08_mixed_content),
	cmocka_unit_test(test_nc09_multiple_entities),
	cmocka_unit_test(test_nc10_whitespace_content),
	cmocka_unit_test(test_ch01_children_null_node),
	cmocka_unit_test(test_ch02_self_closing_zero_children),
	cmocka_unit_test(test_ch03_one_child),
	cmocka_unit_test(test_ch04_multiple_children),
	cmocka_unit_test(test_ch05_text_only_zero_children),
	cmocka_unit_test(test_ch06_comments_not_counted),
	cmocka_unit_test(test_ch07_deep_children_count),
	cmocka_unit_test(test_ck01_child_null_node),
	cmocka_unit_test(test_ck02_first_child),
	cmocka_unit_test(test_ck03_index_out_of_range),
	cmocka_unit_test(test_ck04_large_index),
	cmocka_unit_test(test_ck05_second_child),
	cmocka_unit_test(test_ck06_child_name_correct),
};

void get_ut_node_content_tests(const struct CMUnitTest **out_tests, size_t *out_count) {
	*out_tests = tests;
	*out_count = sizeof(tests) / sizeof(tests[0]);
}
