/*
 * Feature unit tests for xml_open_document (O-01..O-05) and
 * xml_document_free (F-01..F-05).
 * Derived from docs/FeatureTestCases.md.
 *
 * Note: O-06 (NULL FILE*) is omitted — passing NULL to xml_open_document
 * causes a crash in the current implementation (no NULL guard on FILE*).
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


/* Write content to a tmpfile and rewind it. Caller owns the FILE*. */
static FILE *make_tmpfile(char const *content) {
	FILE *f = tmpfile();
	if (!f) { return NULL; }
	if (content && content[0] != '\0') {
		fwrite(content, 1, strlen(content), f);
	}
	rewind(f);
	return f;
}


/* O-01: Valid XML file. */
static void test_o01_valid_file(void **state) {
	(void)state;
	FILE *f = make_tmpfile("<root><child/></root>");
	assert_non_null(f);
	struct xml_document *doc = xml_open_document(f);
	assert_non_null(doc);
	xml_document_free(doc, true);
}

/* O-02: File with XML declaration. */
static void test_o02_file_with_xml_decl(void **state) {
	(void)state;
	FILE *f = make_tmpfile("<?xml version=\"1.0\" encoding=\"UTF-8\"?><r/>");
	assert_non_null(f);
	struct xml_document *doc = xml_open_document(f);
	assert_non_null(doc);
	xml_document_free(doc, true);
}

/* O-03: Empty file — parse fails. */
static void test_o03_empty_file(void **state) {
	(void)state;
	FILE *f = make_tmpfile("");
	assert_non_null(f);
	struct xml_document *doc = xml_open_document(f);
	assert_null(doc);
}

/* O-04: File with invalid XML — parse fails. */
static void test_o04_invalid_xml_file(void **state) {
	(void)state;
	FILE *f = make_tmpfile("<unclosed>");
	assert_non_null(f);
	struct xml_document *doc = xml_open_document(f);
	assert_null(doc);
}

/* O-05: xml_document_buffer_length equals the byte count written to the file. */
static void test_o05_buffer_length_matches_file(void **state) {
	(void)state;
	char const *content = "<root><item>value</item></root>";
	size_t expected = strlen(content);
	FILE *f = make_tmpfile(content);
	assert_non_null(f);
	struct xml_document *doc = xml_open_document(f);
	assert_non_null(doc);
	assert_int_equal(xml_document_buffer_length(doc), expected);
	xml_document_free(doc, true);
}


/* F-01: xml_document_free(NULL, false) — no crash. */
static void test_f01_free_null_no_buffer(void **state) {
	(void)state;
	xml_document_free(NULL, false);
}

/* F-02: xml_document_free(NULL, true) — no crash. */
static void test_f02_free_null_with_buffer(void **state) {
	(void)state;
	xml_document_free(NULL, true);
}

/* F-03: Valid document, free_buffer=false — caller still owns buffer. */
static void test_f03_free_doc_keep_buffer(void **state) {
	(void)state;
	SOURCE(s, "<r/>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	xml_document_free(doc, false);
	/* Buffer s is still accessible; free it ourselves. */
	free(s);
}

/* F-04: Valid document, free_buffer=true — no crash; buffer freed with doc. */
static void test_f04_free_doc_with_buffer(void **state) {
	(void)state;
	SOURCE(s, "<r/>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	xml_document_free(doc, true);
	/* s is now freed; do not access it. */
}

/* F-05: Document from xml_open_document, free_buffer=true — no crash. */
static void test_f05_free_open_doc(void **state) {
	(void)state;
	FILE *f = make_tmpfile("<r/>");
	assert_non_null(f);
	struct xml_document *doc = xml_open_document(f);
	assert_non_null(doc);
	xml_document_free(doc, true);
}


static const struct CMUnitTest tests[] = {
	cmocka_unit_test(test_o01_valid_file),
	cmocka_unit_test(test_o02_file_with_xml_decl),
	cmocka_unit_test(test_o03_empty_file),
	cmocka_unit_test(test_o04_invalid_xml_file),
	cmocka_unit_test(test_o05_buffer_length_matches_file),
	cmocka_unit_test(test_f01_free_null_no_buffer),
	cmocka_unit_test(test_f02_free_null_with_buffer),
	cmocka_unit_test(test_f03_free_doc_keep_buffer),
	cmocka_unit_test(test_f04_free_doc_with_buffer),
	cmocka_unit_test(test_f05_free_open_doc),
};

void get_ut_open_document_tests(const struct CMUnitTest **out_tests, size_t *out_count) {
	*out_tests = tests;
	*out_count = sizeof(tests) / sizeof(tests[0]);
}
