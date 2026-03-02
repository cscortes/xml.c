/**
 * Tests for xml_string_clone behaviour exercised via the public _c_string API
 * (xml_node_name_c_string, xml_node_content_c_string,
 *  xml_node_attribute_name_c_string, xml_node_attribute_content_c_string).
 * Those functions return a newly allocated, null-terminated copy that the
 * caller must free.
 */
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <cmocka.h>
#include <xml.h>
#include "test_runner.h"


/**
 * name_c_string and content_c_string return null-terminated, correct content.
 */
static void test_c_string_name_and_content_null_terminated(void **state) {
	(void)state;
	/* Use a minimal doc so we don't need to track document for free. */
	char const* xml = "<root><elem>content</elem></root>";
	size_t len = strlen(xml);
	uint8_t* buf = malloc(len + 1);
	assert_non_null(buf);
	memcpy(buf, xml, len + 1);
	struct xml_document* doc = xml_parse_document(buf, len);
	assert_non_null(doc);
	struct xml_node* root = xml_document_root(doc);
	struct xml_node* elem = xml_node_child(root, 0);

	uint8_t* name = xml_node_name_c_string(elem);
	assert_non_null(name);
	assert_string_equal((char const*)name, "elem");
	assert_int_equal(strlen((char const*)name), 4);
	free(name);

	uint8_t* content = xml_node_content_c_string(elem);
	assert_non_null(content);
	assert_string_equal((char const*)content, "content");
	assert_int_equal(strlen((char const*)content), 7);
	free(content);

	xml_document_free(doc, true);
}


/**
 * Caller owns the returned buffer; multiple calls return independent copies.
 */
static void test_c_string_caller_owns_buffer(void **state) {
	(void)state;
	char const* xml = "<a>xy</a>";
	size_t len = strlen(xml);
	uint8_t* buf = malloc(len + 1);
	assert_non_null(buf);
	memcpy(buf, xml, len + 1);
	struct xml_document* doc = xml_parse_document(buf, len);
	assert_non_null(doc);
	struct xml_node* root = xml_document_root(doc);

	uint8_t* c1 = xml_node_content_c_string(root);
	assert_non_null(c1);
	assert_string_equal((char const*)c1, "xy");
	free(c1);

	uint8_t* c2 = xml_node_content_c_string(root);
	assert_non_null(c2);
	assert_string_equal((char const*)c2, "xy");
	free(c2);

	xml_document_free(doc, true);
}


/**
 * attribute_name_c_string and attribute_content_c_string return cloned, null-terminated buffers.
 */
static void test_c_string_attribute_cloned(void **state) {
	(void)state;
	char const* xml = "<e id=\"123\" name=\"foo\"></e>";
	size_t len = strlen(xml);
	uint8_t* buf = malloc(len + 1);
	assert_non_null(buf);
	memcpy(buf, xml, len + 1);
	struct xml_document* doc = xml_parse_document(buf, len);
	assert_non_null(doc);
	struct xml_node* root = xml_document_root(doc);

	assert_int_equal(xml_node_attributes(root), 2);

	uint8_t* attr0_name = xml_node_attribute_name_c_string(root, 0);
	uint8_t* attr0_content = xml_node_attribute_content_c_string(root, 0);
	assert_non_null(attr0_name);
	assert_non_null(attr0_content);
	assert_string_equal((char const*)attr0_name, "id");
	assert_string_equal((char const*)attr0_content, "123");
	free(attr0_name);
	free(attr0_content);

	uint8_t* attr1_name = xml_node_attribute_name_c_string(root, 1);
	uint8_t* attr1_content = xml_node_attribute_content_c_string(root, 1);
	assert_non_null(attr1_name);
	assert_non_null(attr1_content);
	assert_string_equal((char const*)attr1_name, "name");
	assert_string_equal((char const*)attr1_content, "foo");
	free(attr1_name);
	free(attr1_content);

	xml_document_free(doc, true);
}


/**
 * Empty attribute value: attribute_content_c_string returns a valid buffer (empty string).
 */
static void test_c_string_attribute_empty_value(void **state) {
	(void)state;
	char const* xml = "<e empty=\"\"></e>";
	size_t len = strlen(xml);
	uint8_t* buf = malloc(len + 1);
	assert_non_null(buf);
	memcpy(buf, xml, len + 1);
	struct xml_document* doc = xml_parse_document(buf, len);
	assert_non_null(doc);
	struct xml_node* root = xml_document_root(doc);

	uint8_t* content = xml_node_attribute_content_c_string(root, 0);
	assert_non_null(content);
	assert_string_equal((char const*)content, "");
	assert_int_equal(strlen((char const*)content), 0);
	free(content);

	xml_document_free(doc, true);
}


/**
 * NULL node: all _c_string getters return NULL (no clone attempted).
 * (Also covered in unit-c-null.c; this file focuses on clone behaviour.)
 */
static void test_c_string_null_node_returns_null(void **state) {
	(void)state;
	assert_null(xml_node_name_c_string(NULL));
	assert_null(xml_node_content_c_string(NULL));
	assert_null(xml_node_attribute_name_c_string(NULL, 0));
	assert_null(xml_node_attribute_content_c_string(NULL, 0));
}


static const struct CMUnitTest tests[] = {
	cmocka_unit_test(test_c_string_name_and_content_null_terminated),
	cmocka_unit_test(test_c_string_caller_owns_buffer),
	cmocka_unit_test(test_c_string_attribute_cloned),
	cmocka_unit_test(test_c_string_attribute_empty_value),
	cmocka_unit_test(test_c_string_null_node_returns_null),
};

void get_unit_c_string_clone_tests(const struct CMUnitTest** out_tests, size_t* out_count) {
	*out_tests = tests;
	*out_count = sizeof(tests) / sizeof(tests[0]);
}
