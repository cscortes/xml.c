/**
 * Copyright (c) 2012 ooxi/xml.c
 *     https://github.com/ooxi/xml.c
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from the
 * use of this software.
 * 
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 *  1. The origin of this software must not be misrepresented; you must not
 *     claim that you wrote the original software. If you use this software in a
 *     product, an acknowledgment in the product documentation would be
 *     appreciated but is not required.
 * 
 *  2. Altered source versions must be plainly marked as such, and must not be
 *     misrepresented as being the original software.
 *
 *  3. This notice may not be removed or altered from any source distribution.
 */
#include <stdbool.h>
#include <stddef.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <cmocka.h>
#include <xml.h>
#include "test_runner.h"


/**
 * @return true iff xml string equals the C string
 */
static bool string_equals(struct xml_string* a, char const* b) {
	if (!a) {
		return b == NULL || b[0] == '\0';
	}
	size_t a_length = xml_string_length(a);
	size_t b_length = strlen(b);

	uint8_t* a_buffer = malloc((a_length + 1) * sizeof(uint8_t));
	if (!a_buffer)
		return false;
	xml_string_copy(a, a_buffer, a_length);
	a_buffer[a_length] = 0;

	bool equal = (a_length == b_length && memcmp(a_buffer, b, a_length) == 0);
	free(a_buffer);
	return equal;
}


/**
 * Converts a static character array to an uint8_t data source
 */
#define SOURCE(source, content)						\
	uint8_t* source = calloc(strlen(content) + 1, sizeof(uint8_t));	\
	{	char const* content_string = content;			\
		memcpy(source, content_string, strlen(content) + 1);	\
	}


/**
 * Depth-first search for first node with given tag name below base.
 * Similar to getElementsByTagName but returns only the first match.
 *
 * @param base Node at which to start the search
 * @param name 0-terminated tag name (case sensitive)
 * @return First node below base with that tag name, or NULL if not found
 *
 * @see https://developer.mozilla.org/en-US/docs/Web/API/Element/getElementsByTagName
 */
static struct xml_node* get_node_by_name(struct xml_node* base, char const* name) {
	size_t const name_length = strlen(name);

	uint8_t* base_name = xml_easy_name(base);
	size_t const base_name_length = strlen((char const*)base_name);

	if (name_length == base_name_length) {
		if (memcmp(name, base_name, name_length) == 0) {
			free(base_name);
			return base;
		}
	}
	free(base_name);

	size_t const number_of_children = xml_node_children(base);
	if (!number_of_children)
		return NULL;

	for (size_t child = 0; child < number_of_children; ++child) {
		struct xml_node* child_node = xml_node_child(base, child);
		struct xml_node* search_result = get_node_by_name(child_node, name);
		if (search_result)
			return search_result;
	}
	return NULL;
}


/**
 * Tries to parse a simple document containing only one tag
 */
static void test_xml_parse_document_0(void **state) {
	(void)state;
	SOURCE(source, "<Hello>World</Hello>");

	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	assert_non_null(document);

	struct xml_node* root = xml_document_root(document);
	assert_true(string_equals(xml_node_name(root), "Hello"));
	assert_true(string_equals(xml_node_content(root), "World"));

	xml_document_free(document, true);
}


/**
 * Tries to parse a document containing multiple tags
 */
static void test_xml_parse_document_1(void **state) {
	(void)state;
	SOURCE(source, ""
		"<Parent>\n"
		"\t<Child>\n"
		"\t\tFirst content\n"
		"\t</Child>\n"
		"\t<Child>\n"
		"\t\tSecond content\n"
		"\t</Child>\n"
		"</Parent>\n"
	);
	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	assert_non_null(document);

	struct xml_node* root = xml_document_root(document);
	assert_true(string_equals(xml_node_name(root), "Parent"));
	assert_int_equal(xml_node_children(root), 2);

	struct xml_node* first_child = xml_node_child(root, 0);
	struct xml_node* second_child = xml_node_child(root, 1);
	assert_non_null(first_child);
	assert_non_null(second_child);

	struct xml_node* third_child = xml_node_child(root, 2);
	assert_null(third_child);

	assert_true(string_equals(xml_node_name(first_child), "Child"));
	assert_true(string_equals(xml_node_content(first_child), "First content"));
	assert_true(string_equals(xml_node_name(second_child), "Child"));
	assert_true(string_equals(xml_node_content(second_child), "Second content"));

	xml_document_free(document, true);
}


/**
 * Tests the easy functionality (xml_easy_child, xml_easy_name, xml_easy_content)
 */
static void test_xml_parse_document_2(void **state) {
	(void)state;
	SOURCE(source, ""
		"<Parent>\n"
		"\t<Child>\n"
		"\t\tFirst content\n"
		"\t</Child>\n"
		"\t<This><Is>\n"
			"<A><Test>Content A</Test></A>\n"
			"<B><Test>Content B</Test></B>\n"
		"\t</Is></This>\n"
		"\t<Child>\n"
		"\t\tSecond content\n"
		"\t</Child>\n"
		"</Parent>\n"
	);
	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	assert_non_null(document);

	struct xml_node* root = xml_document_root(document);
	assert_true(string_equals(xml_node_name(root), "Parent"));
	assert_int_equal(xml_node_children(root), 3);

	struct xml_node* test_a = xml_easy_child(root, (uint8_t const*)"This", (uint8_t const*)"Is", (uint8_t const*)"A", (uint8_t const*)"Test", 0);
	assert_non_null(test_a);
	assert_true(string_equals(xml_node_content(test_a), "Content A"));

	struct xml_node* test_b = xml_easy_child(root, (uint8_t const*)"This", (uint8_t const*)"Is", (uint8_t const*)"B", (uint8_t const*)"Test", 0);
	assert_non_null(test_b);
	assert_true(string_equals(xml_node_content(test_b), "Content B"));

	struct xml_node* test_c = xml_easy_child(root, (uint8_t const*)"This", (uint8_t const*)"Is", (uint8_t const*)"C", (uint8_t const*)"Test", 0);
	assert_null(test_c);

	struct xml_node* must_be_null = xml_easy_child(root, (uint8_t const*)"Child");
	assert_null(must_be_null);

	uint8_t* name_is = xml_easy_name(xml_easy_child(root, (uint8_t const*)"This", (uint8_t const*)"Is", 0));
	assert_string_equal((char const*)name_is, "Is");
	free(name_is);

	uint8_t* content_a = xml_easy_content(test_a);
	assert_string_equal((char const*)content_a, "Content A");
	free(content_a);

	xml_document_free(document, true);
}


/**
 * Tests the xml_open_document functionality
 */
static void test_xml_parse_document_3(void **state) {
	(void)state;
	#define FILE_NAME "input/test.xml"
	FILE* handle = fopen(FILE_NAME, "rb");
	assert_non_null(handle);

	struct xml_document* document = xml_open_document(handle);
	assert_non_null(document);

	struct xml_node* element = xml_easy_child(
		xml_document_root(document), (uint8_t const*)"Element", (uint8_t const*)"With", 0
	);
	assert_non_null(element);
	assert_true(string_equals(xml_node_content(element), "Child"));

	xml_document_free(document, true);
	#undef FILE_NAME
}


/**
 * xml_open_document from a temp file with exactly 8 bytes "<a></a>".
 * Asserts document buffer length is exactly 8 and root is "a". Catches feof-style
 * off-by-one (extra read would grow length or corrupt content) and verifies
 * fread-driven loop and fclose path.
 */
static void test_open_document_temp_file_exact_bytes(void **state) {
	(void)state;
	FILE* f = tmpfile();
	assert_non_null(f);
	static char const content[] = "<a></a>";
	size_t n = strlen(content);
	assert_true(fwrite(content, 1, n, f) == n);
	rewind(f);

	struct xml_document* document = xml_open_document(f);
	assert_non_null(document);
	struct xml_node* root = xml_document_root(document);
	assert_non_null(root);
	assert_true(string_equals(xml_node_name(root), "a"));
	assert_int_equal(xml_document_buffer_length(document), n);

	xml_document_free(document, true);
	/* f was closed by xml_open_document; tmpfile() stream is already gone
	 */
}


/**
 * xml_open_document with a minimal known-size file (exact buffer).
 * Parses input/minimal.xml (content "<a></a>") and asserts root name is "a",
 * no extra content (empty element, no children), and document buffer length
 * equals file size. Exercises the feof/read path; a classic feof() off-by-one
 * could yield wrong length or extra bytes.
 */
static void test_open_document_exact_buffer(void **state) {
	(void)state;
	FILE* f = fopen("input/minimal.xml", "rb");
	assert_non_null(f);
	int seek_ok = fseek(f, 0, SEEK_END);
	assert_int_equal(seek_ok, 0);
	long file_size = ftell(f);
	assert_true(file_size >= 0);
	rewind(f);

	struct xml_document* document = xml_open_document(f);
	assert_non_null(document);

	struct xml_node* root = xml_document_root(document);
	assert_non_null(root);
	assert_true(string_equals(xml_node_name(root), "a"));
	assert_int_equal(xml_node_children(root), 0);
	assert_true(xml_document_buffer_length(document) == (size_t)file_size);

	xml_document_free(document, true);
}


/**
 * xml_open_document on an empty file must return NULL (no read past end, no crash).
 * Exercises the fread loop when first read returns 0 and ferror/fclose paths.
 */
static void test_open_document_empty_file(void **state) {
	(void)state;
	FILE* f = tmpfile();
	assert_non_null(f);
	/* Write nothing; position at 0, length 0
	 */
	rewind(f);

	struct xml_document* document = xml_open_document(f);
	assert_null(document);
	/* f was closed by xml_open_document
	 */
}


/**
 * Parse with exact length (no trailing NUL in count). Ensures we do not read past
 * the given length: success case "<r></r>" with length 8, failure case "<r>" with
 * length 3. With the consume-at-end fix (position = length, no clamp to length-1),
 * code that checks position < length before reading stays in bounds; Valgrind/ASan
 * would catch any overread.
 */
static void test_parse_exact_length_boundary(void **state) {
	(void)state;
	SOURCE(src_ok, "<r></r>");
	size_t len_ok = strlen((char const*)src_ok);

	struct xml_document* doc = xml_parse_document(src_ok, len_ok);
	assert_non_null(doc);
	struct xml_node* root = xml_document_root(doc);
	assert_non_null(root);
	assert_true(string_equals(xml_node_name(root), "r"));
	assert_int_equal(xml_node_children(root), 0);
	xml_document_free(doc, true);

	SOURCE(src_bad, "<r>");
	size_t len_bad = strlen((char const*)src_bad);
	struct xml_document* doc_fail = xml_parse_document(src_bad, len_bad);
	assert_null(doc_fail);
	free(src_bad);
}


/**
 * Attributes from in-memory buffer: node with 0 attributes.
 * xml_parse_document(source, len); no attributes on first child.
 */
static void test_attributes_in_memory_0(void **state) {
	(void)state;
	SOURCE(source, "<Root><Node>text</Node></Root>");

	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	assert_non_null(document);

	struct xml_node* root = xml_document_root(document);
	struct xml_node* element = xml_node_child(root, 0);

	assert_non_null(element);
	assert_int_equal(xml_node_attributes(element), 0);

	xml_document_free(document, true);
}


/**
 * Attributes from in-memory buffer: node with 1 attribute.
 * Fails when parser stops at first space (docs/testable_issues_priority.md §2).
 */
static void test_attributes_in_memory_1(void **state) {
	(void)state;
	SOURCE(source, "<Root><Node attr=\"value\">text</Node></Root>");

	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	assert_non_null(document);

	struct xml_node* root = xml_document_root(document);
	struct xml_node* element = xml_node_child(root, 0);

	assert_non_null(element);
	assert_int_equal(xml_node_attributes(element), 1);
	assert_true(string_equals(xml_node_attribute_name(element, 0), "attr"));
	assert_true(string_equals(xml_node_attribute_content(element, 0), "value"));

	xml_document_free(document, true);
}


/**
 * Attributes from in-memory buffer: node with 2 attributes.
 * Fails when parser stops at first space (docs/testable_issues_priority.md §2).
 */
static void test_attributes_in_memory_2(void **state) {
	(void)state;
	SOURCE(source, "<Root><Node a=\"1\" b=\"2\">text</Node></Root>");

	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	assert_non_null(document);

	struct xml_node* root = xml_document_root(document);
	struct xml_node* element = xml_node_child(root, 0);

	assert_non_null(element);
	assert_int_equal(xml_node_attributes(element), 2);
	assert_true(string_equals(xml_node_attribute_name(element, 0), "a"));
	assert_true(string_equals(xml_node_attribute_content(element, 0), "1"));
	assert_true(string_equals(xml_node_attribute_name(element, 1), "b"));
	assert_true(string_equals(xml_node_attribute_content(element, 1), "2"));

	xml_document_free(document, true);
}


/**
 * Attributes from file: node with 0 attributes.
 * xml_open_document(handle); input/test-attributes-0.xml.
 */
static void test_attributes_from_file_0(void **state) {
	(void)state;
	FILE* handle = fopen("input/test-attributes-0.xml", "rb");
	assert_non_null(handle);

	struct xml_document* document = xml_open_document(handle);
	assert_non_null(document);

	struct xml_node* root = xml_document_root(document);
	struct xml_node* element = xml_node_child(root, 0);

	assert_non_null(element);
	assert_int_equal(xml_node_attributes(element), 0);

	xml_document_free(document, true);
}


/**
 * Attributes from file: node with 1 attribute.
 * input/test-attributes-1.xml. Fails when parser stops at first space.
 */
static void test_attributes_from_file_1(void **state) {
	(void)state;
	FILE* handle = fopen("input/test-attributes-1.xml", "rb");
	assert_non_null(handle);

	struct xml_document* document = xml_open_document(handle);
	assert_non_null(document);

	struct xml_node* root = xml_document_root(document);
	struct xml_node* element = xml_node_child(root, 0);

	assert_non_null(element);
	assert_int_equal(xml_node_attributes(element), 1);
	assert_true(string_equals(xml_node_attribute_name(element, 0), "id"));
	assert_true(string_equals(xml_node_attribute_content(element, 0), "one"));

	xml_document_free(document, true);
}


/**
 * Attributes from file: node with 2 attributes.
 * input/test-attributes-2.xml. Fails when parser stops at first space.
 *
 * @author Isty001
 * @see https://github.com/Isty001/
 */
static void test_attributes_from_file_2(void **state) {
	(void)state;
	FILE* handle = fopen("input/test-attributes-2.xml", "rb");
	assert_non_null(handle);

	struct xml_document* document = xml_open_document(handle);
	assert_non_null(document);

	struct xml_node* root = xml_document_root(document);
	struct xml_node* element = xml_node_child(root, 0);

	assert_non_null(element);
	assert_int_equal(xml_node_attributes(element), 2);
	assert_true(string_equals(xml_node_attribute_name(element, 0), "value"));
	assert_true(string_equals(xml_node_attribute_content(element, 0), "2"));
	assert_true(string_equals(xml_node_attribute_name(element, 1), "value_2"));
	assert_true(string_equals(xml_node_attribute_content(element, 1), "Hello"));

	xml_document_free(document, true);
}


/**
 * Tiled/SVG-style XML: opening tag spans multiple lines with attributes.
 * Upstream #38 reported parse errors on such input; the fix was to parse the
 * full opening tag up to '>' (and any newline handling in attribute parsing).
 * This test would have caught that regression: parse must succeed and yield
 * the element with its attributes and child.
 *
 * @see docs/issues.md (#38, #39 part, #33)
 */
static void test_tiled_svg_style_multiline_opening_tag(void **state) {
	(void)state;
	SOURCE(source, ""
		"<Root>\n"
		"<Element\n"
		"  id=\"one\"\n"
		"  name=\"foo\">\n"
		"  <Child>content</Child>\n"
		"</Element>\n"
		"</Root>\n"
	);

	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	if (!document) {
		free(source);
		assert_non_null(document);
	}

	struct xml_node* root = xml_document_root(document);
	assert_non_null(root);
	assert_true(string_equals(xml_node_name(root), "Root"));

	struct xml_node* element = xml_node_child(root, 0);
	assert_non_null(element);
	assert_true(string_equals(xml_node_name(element), "Element"));
	assert_int_equal(xml_node_attributes(element), 2);
	assert_true(string_equals(xml_node_attribute_name(element, 0), "id"));
	assert_true(string_equals(xml_node_attribute_content(element, 0), "one"));
	assert_true(string_equals(xml_node_attribute_name(element, 1), "name"));
	assert_true(string_equals(xml_node_attribute_content(element, 1), "foo"));

	struct xml_node* child = xml_node_child(element, 0);
	assert_non_null(child);
	assert_true(string_equals(xml_node_name(child), "Child"));
	assert_true(string_equals(xml_node_content(child), "content"));

	xml_document_free(document, true);
}


/**
 * Tests finding a node by tag name (depth-first, first match).
 */
static void test_find_node_by_tag_name(void **state) {
	(void)state;
	SOURCE(source, ""
		"<Root>"
			"<Hello>World</Hello>"
			"<Functions>"
				"<Function>"
					"<as>testas one</as>"
					"<os>testos</os>"
				"</Function>"
				"<Function>"
					"<is>testis</is>"
					"<us>testus</us>"
					"<ls>testls</ls>"
				"</Function>"
				"<Function>"
					"<mn>testmn</mn>"
					"<as>testas two</as>"
				"</Function>"
			"</Functions>"
		"</Root>"
	);

	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	assert_non_null(document);
	struct xml_node* root = xml_document_root(document);

	struct xml_node* us = get_node_by_name(root, "us");
	assert_non_null(us);
	uint8_t* us_content = xml_easy_content(us);
	assert_non_null(us_content);
	assert_string_equal((char const*)us_content, "testus");
	free(us_content);

	struct xml_node* as = get_node_by_name(root, "as");
	assert_non_null(as);
	uint8_t* as_content = xml_easy_content(as);
	assert_non_null(as_content);
	assert_string_equal((char const*)as_content, "testas one");
	free(as_content);

	struct xml_node* does_not_exist = get_node_by_name(root, "does_not_exist");
	assert_null(does_not_exist);

	xml_document_free(document, true);
}


/**
 * Malformed XML that triggers a parse error at end of buffer (e.g. no closing tag).
 * Expect NULL document. When run under ASan, ensures xml_parser_error does not
 * read past end of buffer when reporting the error.
 * stderr is redirected during the parse so expected parser error messages
 * do not appear in the test output.
 */
static void test_parse_error_at_end_of_buffer(void **state) {
	(void)state;
	SOURCE(src, "<root>");

	int saved_stderr = dup(STDERR_FILENO);
	if (saved_stderr >= 0) {
		int devnull = open("/dev/null", O_WRONLY);
		if (devnull >= 0) {
			dup2(devnull, STDERR_FILENO);
			close(devnull);
		}
	}

	struct xml_document* doc = xml_parse_document(src, (size_t)strlen((char const*)src));

	if (saved_stderr >= 0) {
		dup2(saved_stderr, STDERR_FILENO);
		close(saved_stderr);
	}

	assert_null(doc);
	free(src);
}


/**
 * When realloc fails (returns NULL), parser must not leak and must not double-free.
 * Uses cmocka + linker --wrap=realloc so the next realloc returns NULL.
 * Parsing "<r><c/></r>" triggers a realloc when adding the first child; we make that fail.
 * stderr is redirected to /dev/null during the parse so expected parser error messages
 * do not appear in the test output and confuse new developers.
 */
static void test_realloc_failure_no_leak(void **state) {
	(void)state;
	will_return_ptr(__wrap_realloc, NULL);
	SOURCE(src, "<r><c/></r>");

	int saved_stderr = dup(STDERR_FILENO);
	if (saved_stderr >= 0) {
		int devnull = open("/dev/null", O_WRONLY);
		if (devnull >= 0) {
			dup2(devnull, STDERR_FILENO);
			close(devnull);
		}
	}

	struct xml_document* doc = xml_parse_document(src, (size_t)strlen((char const*)src));

	if (saved_stderr >= 0) {
		dup2(saved_stderr, STDERR_FILENO);
		close(saved_stderr);
	}

	assert_null(doc);
	free(src);
}


	static const struct CMUnitTest tests[] = {
	cmocka_unit_test(test_xml_parse_document_0),
	cmocka_unit_test(test_xml_parse_document_1),
	cmocka_unit_test(test_xml_parse_document_2),
	cmocka_unit_test(test_xml_parse_document_3),
	cmocka_unit_test(test_open_document_temp_file_exact_bytes),
	cmocka_unit_test(test_open_document_exact_buffer),
	cmocka_unit_test(test_open_document_empty_file),
	cmocka_unit_test(test_parse_exact_length_boundary),
	cmocka_unit_test(test_attributes_in_memory_0),
	cmocka_unit_test(test_attributes_in_memory_1),
	cmocka_unit_test(test_attributes_in_memory_2),
	cmocka_unit_test(test_attributes_from_file_0),
	cmocka_unit_test(test_attributes_from_file_1),
	cmocka_unit_test(test_attributes_from_file_2),
	cmocka_unit_test(test_tiled_svg_style_multiline_opening_tag),
	cmocka_unit_test(test_find_node_by_tag_name),
	cmocka_unit_test(test_parse_error_at_end_of_buffer),
	cmocka_unit_test(test_realloc_failure_no_leak),
};

void get_unit_c_tests(const struct CMUnitTest** out_tests, size_t* out_count) {
	*out_tests = tests;
	*out_count = sizeof(tests) / sizeof(tests[0]);
}
