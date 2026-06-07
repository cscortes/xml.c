/**
 * Tests for xml_scan_word exercised via the public xml_parse_document API.
 *
 * xml_scan_word extracts the first whitespace-delimited word from a mutable
 * string, used once per tag to separate the tag name from its attributes
 * (e.g. "root id=\"1\"" → word "root", remainder "id=\"1\"").
 *
 * Branches covered:
 *   (a) word at end of string   — inner loop hits '\0', no null-termination
 *   (b) word followed by space  — inner loop hits whitespace, null-terminates
 *   (c) leading whitespace      — outer loop advances str before word search
 *   (d) all whitespace / empty  — outer loop reaches '\0', returns NULL
 */
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <cmocka.h>
#include <xml.h>
#include "test_runner.h"


/* Branch (a): word reaches end of string — tag with name only, no attributes.
 * xml_scan_word("root", &rest): inner loop hits '\0', if (*str) is false,
 * nextp is set to the null terminator. */
static void test_scan_word_at_end_of_string(void **state) {
	(void)state;
	const char *xml = "<root></root>";
	size_t len = strlen(xml);
	uint8_t *buf = malloc(len + 1);
	assert_non_null(buf);
	memcpy(buf, xml, len + 1);

	struct xml_document *doc = xml_parse_document(buf, len);
	assert_non_null(doc);

	struct xml_node *root = xml_document_root(doc);
	assert_non_null(root);

	struct xml_string *name = xml_node_name(root);
	assert_non_null(name);
	assert_int_equal(xml_string_equals_cstr(name, (uint8_t const*)"root"), 1);

	xml_document_free(doc, true);
}


/* Branch (b): word followed by whitespace — tag name with an attribute.
 * xml_scan_word("root id=\"1\"", &rest): inner loop stops at the space after
 * "root", *str is non-NUL so *str++ = '\0' null-terminates and advances nextp. */
static void test_scan_word_followed_by_whitespace(void **state) {
	(void)state;
	const char *xml = "<root id=\"1\"></root>";
	size_t len = strlen(xml);
	uint8_t *buf = malloc(len + 1);
	assert_non_null(buf);
	memcpy(buf, xml, len + 1);

	struct xml_document *doc = xml_parse_document(buf, len);
	assert_non_null(doc);

	struct xml_node *root = xml_document_root(doc);
	assert_non_null(root);

	struct xml_string *name = xml_node_name(root);
	assert_non_null(name);
	assert_int_equal(xml_string_equals_cstr(name, (uint8_t const*)"root"), 1);
	assert_int_equal(xml_node_attributes(root), 1);

	xml_document_free(doc, true);
}


/* Branch (c): leading whitespace skipped — tag_open starts with a space.
 * xml_scan_word(" root", &rest): outer loop skips the leading space.
 * The downstream offset mismatch causes xml_validate_tag_name to reject the
 * tag (buffer[0] is ' ', not a valid NameStartChar), so the doc is NULL. */
static void test_scan_word_leading_whitespace_causes_parse_error(void **state) {
	(void)state;
	/* Space immediately after '<' puts leading whitespace in tag_open. */
	const char *xml = "< root></ root>";
	size_t len = strlen(xml);
	uint8_t *buf = malloc(len + 1);
	assert_non_null(buf);
	memcpy(buf, xml, len + 1);

	struct xml_document *doc = xml_parse_document(buf, len);
	assert_null(doc);

	free(buf);
}


/* Branch (d): all whitespace — tag_open contains only spaces.
 * xml_scan_word("   ", &rest): outer loop reaches '\0', returns NULL.
 * xml_find_attributes jumps to cleanup; tag_open->length stays 3 and
 * xml_validate_tag_name rejects it, so the doc is NULL. */
static void test_scan_word_all_whitespace_returns_null(void **state) {
	(void)state;
	const char *xml = "<   ></   >";
	size_t len = strlen(xml);
	uint8_t *buf = malloc(len + 1);
	assert_non_null(buf);
	memcpy(buf, xml, len + 1);

	struct xml_document *doc = xml_parse_document(buf, len);
	assert_null(doc);

	free(buf);
}


static const struct CMUnitTest tests[] = {
	cmocka_unit_test(test_scan_word_at_end_of_string),
	cmocka_unit_test(test_scan_word_followed_by_whitespace),
	cmocka_unit_test(test_scan_word_leading_whitespace_causes_parse_error),
	cmocka_unit_test(test_scan_word_all_whitespace_returns_null),
};

void get_unit_c_strtok_tests(const struct CMUnitTest **out_tests, size_t *out_count) {
	*out_tests = tests;
	*out_count = sizeof(tests) / sizeof(tests[0]);
}
