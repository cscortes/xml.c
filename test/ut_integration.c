/*
 * Feature unit tests — cross-API integration scenarios (I-01..I-12).
 * Derived from docs/FeatureTestCases.md.
 *
 * Run with:  ctest -L unit
 */
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cmocka.h>
#include <xml.h>
#include "ut_helpers.h"
#include "ut_runner.h"


/* I-01: Parse → root → name → equals_cstr */
static void test_i01_parse_root_name_equals_cstr(void **state) {
	(void)state;
	SOURCE(s, "<document></document>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	struct xml_node *root = xml_document_root(doc);
	assert_non_null(root);
	assert_true(xml_string_equals_cstr(xml_node_name(root), (uint8_t const *)"document"));
	xml_document_free(doc, true);
}

/* I-02: Parse → root → child(0) → name */
static void test_i02_parse_root_child_name(void **state) {
	(void)state;
	SOURCE(s, "<r><item></item></r>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	struct xml_node *root = xml_document_root(doc);
	assert_int_equal(xml_node_children(root), 1);
	struct xml_node *item = xml_node_child(root, 0);
	assert_non_null(item);
	assert_true(xml_string_equals_cstr(xml_node_name(item), (uint8_t const *)"item"));
	xml_document_free(doc, true);
}

/* I-03: Parse → easy_child path → content_c_string */
static void test_i03_easy_child_content_cstr(void **state) {
	(void)state;
	SOURCE(s, "<config><host>localhost</host></config>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	struct xml_node *host = xml_easy_child(xml_document_root(doc),
	                                       (uint8_t const *)"host",
	                                       (uint8_t const *)0);
	assert_non_null(host);
	uint8_t *cstr = xml_node_content_c_string(host);
	assert_non_null(cstr);
	assert_string_equal((char const *)cstr, "localhost");
	free(cstr);
	xml_document_free(doc, true);
}

/* I-04: Open file → buffer_length matches file size */
static void test_i04_open_file_buffer_length(void **state) {
	(void)state;
	char const *content = "<r><item>value</item></r>";
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

/* I-05: Parse → attribute_name_c_string + attribute_content_c_string at same index */
static void test_i05_attr_name_and_content_cstr(void **state) {
	(void)state;
	SOURCE(s, "<r lang=\"en\"/>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	struct xml_node *root = xml_document_root(doc);
	uint8_t *name_cstr = xml_node_attribute_name_c_string(root, 0);
	uint8_t *val_cstr  = xml_node_attribute_content_c_string(root, 0);
	assert_non_null(name_cstr);
	assert_non_null(val_cstr);
	assert_string_equal((char const *)name_cstr, "lang");
	assert_string_equal((char const *)val_cstr,  "en");
	free(name_cstr);
	free(val_cstr);
	xml_document_free(doc, true);
}

/* I-06: Parse → string_length + string_copy + manual compare */
static void test_i06_string_length_and_copy(void **state) {
	(void)state;
	SOURCE(s, "<r>world</r>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	struct xml_string *content = xml_node_content(xml_document_root(doc));
	assert_non_null(content);
	size_t len = xml_string_length(content);
	assert_int_equal(len, 5);
	uint8_t buf[6];
	xml_string_copy(content, buf, len);
	buf[len] = '\0';
	assert_memory_equal(buf, "world", 5);
	xml_document_free(doc, true);
}

/* I-07: Parse → string_equals on two nodes with same tag name */
static void test_i07_string_equals_same_tag(void **state) {
	(void)state;
	SOURCE(s, "<r><tag/><tag/></r>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	struct xml_node *root = xml_document_root(doc);
	struct xml_string *n1 = xml_node_name(xml_node_child(root, 0));
	struct xml_string *n2 = xml_node_name(xml_node_child(root, 1));
	assert_true(xml_string_equals(n1, n2));
	xml_document_free(doc, true);
}

/* I-08: Parse with entity → content_c_string → verify expansion */
static void test_i08_entity_content_cstr(void **state) {
	(void)state;
	SOURCE(s, "<r>&lt;b&gt;bold&lt;/b&gt;</r>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	uint8_t *cstr = xml_node_content_c_string(xml_document_root(doc));
	assert_non_null(cstr);
	assert_string_equal((char const *)cstr, "<b>bold</b>");
	free(cstr);
	xml_document_free(doc, true);
}

/* I-09: Parse CDATA → content → equals_cstr */
static void test_i09_cdata_content_equals_cstr(void **state) {
	(void)state;
	SOURCE(s, "<r><![CDATA[raw & <data>]]></r>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	struct xml_string *content = xml_node_content(xml_document_root(doc));
	assert_non_null(content);
	assert_true(xml_string_equals_cstr(content, (uint8_t const *)"raw & <data>"));
	xml_document_free(doc, true);
}

/* I-10: Parse namespace → attribute loop → name equals "xmlns:ns" */
static void test_i10_namespace_attr_enumeration(void **state) {
	(void)state;
	SOURCE(s, "<r xmlns:ns=\"http://example.com\" id=\"1\"/>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	struct xml_node *root = xml_document_root(doc);
	size_t count = xml_node_attributes(root);
	assert_int_equal(count, 2);
	bool found = false;
	for (size_t i = 0; i < count; i++) {
		if (xml_string_equals_cstr(xml_node_attribute_name(root, i),
		                           (uint8_t const *)"xmlns:ns")) {
			found = true;
			break;
		}
	}
	assert_true(found);
	xml_document_free(doc, true);
}

/* I-11: xml_document_free(doc, false) — buffer outlives document. */
static void test_i11_free_keep_buffer(void **state) {
	(void)state;
	SOURCE(s, "<r>data</r>");
	size_t len = strlen((char const *)s);
	struct xml_document *doc = xml_parse_document(s, len);
	assert_non_null(doc);
	xml_document_free(doc, false);
	/* s is still accessible after the document is freed. */
	assert_int_equal(s[0], '<');
	free(s);
}

/* I-12: xml_document_free(doc, true) — buffer freed with document; do not access after. */
static void test_i12_free_with_buffer(void **state) {
	(void)state;
	SOURCE(s, "<r/>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	xml_document_free(doc, true);
	/* s is freed; we do not access it here. */
}


static const struct CMUnitTest tests[] = {
	cmocka_unit_test(test_i01_parse_root_name_equals_cstr),
	cmocka_unit_test(test_i02_parse_root_child_name),
	cmocka_unit_test(test_i03_easy_child_content_cstr),
	cmocka_unit_test(test_i04_open_file_buffer_length),
	cmocka_unit_test(test_i05_attr_name_and_content_cstr),
	cmocka_unit_test(test_i06_string_length_and_copy),
	cmocka_unit_test(test_i07_string_equals_same_tag),
	cmocka_unit_test(test_i08_entity_content_cstr),
	cmocka_unit_test(test_i09_cdata_content_equals_cstr),
	cmocka_unit_test(test_i10_namespace_attr_enumeration),
	cmocka_unit_test(test_i11_free_keep_buffer),
	cmocka_unit_test(test_i12_free_with_buffer),
};

void get_ut_integration_tests(const struct CMUnitTest **out_tests, size_t *out_count) {
	*out_tests = tests;
	*out_count = sizeof(tests) / sizeof(tests[0]);
}
