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

	uint8_t* base_name = xml_node_name_c_string(base);
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
 * Tests path child (xml_easy_child) and C-string helpers (xml_node_name_c_string, xml_node_content_c_string)
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

	uint8_t* name_is = xml_node_name_c_string(xml_easy_child(root, (uint8_t const*)"This", (uint8_t const*)"Is", 0));
	assert_string_equal((char const*)name_is, "Is");
	free(name_is);

	uint8_t* content_a = xml_node_content_c_string(test_a);
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
 * stderr is redirected during the call so expected xml_parser_error output
 * (e.g. "length equals zero") does not appear in the test report.
 */
static void test_open_document_empty_file(void **state) {
	(void)state;
	FILE* f = tmpfile();
	assert_non_null(f);
	/* Write nothing; position at 0, length 0
	 */
	rewind(f);

	int saved_stderr = dup(STDERR_FILENO);
	if (saved_stderr >= 0) {
		int devnull = open("/dev/null", O_WRONLY);
		if (devnull >= 0) {
			dup2(devnull, STDERR_FILENO);
			close(devnull);
		}
	}

	struct xml_document* document = xml_open_document(f);
	assert_null(document);

	if (saved_stderr >= 0) {
		dup2(saved_stderr, STDERR_FILENO);
		close(saved_stderr);
	}
	/* f was closed by xml_open_document
	 */
}


/**
 * Parse with exact length (no trailing NUL in count). Ensures we do not read past
 * the given length: success case "<r></r>" with length 8, failure case "<r>" with
 * length 3. With the consume-at-end fix (position = length, no clamp to length-1),
 * code that checks position < length before reading stays in bounds; Valgrind/ASan
 * would catch any overread.
 * stderr is redirected during the failing parse so expected xml_parser_error output
 * does not appear in the test report.
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

	int saved_stderr = dup(STDERR_FILENO);
	if (saved_stderr >= 0) {
		int devnull = open("/dev/null", O_WRONLY);
		if (devnull >= 0) {
			dup2(devnull, STDERR_FILENO);
			close(devnull);
		}
	}

	struct xml_document* doc_fail = xml_parse_document(src_bad, len_bad);

	if (saved_stderr >= 0) {
		dup2(saved_stderr, STDERR_FILENO);
		close(saved_stderr);
	}

	assert_null(doc_fail);
	free(src_bad);
}


/**
 * xml_parse_document with length 0 must return NULL (no read, no crash).
 * Spec category 3: empty buffer.
 */
static void test_parse_empty_buffer(void **state) {
	(void)state;
	static uint8_t empty[] = { 0 };
	struct xml_document* doc = xml_parse_document(empty, 0);
	assert_null(doc);
}


/**
 * Malformed XML must yield NULL document (category 3).
 * Truncated tag and wrong closing tag; expect NULL.
 */
static void test_parse_malformed_returns_null(void **state) {
	(void)state;
	int saved_stderr = dup(STDERR_FILENO);
	if (saved_stderr >= 0) {
		int devnull = open("/dev/null", O_WRONLY);
		if (devnull >= 0) {
			dup2(devnull, STDERR_FILENO);
			close(devnull);
		}
	}

	SOURCE(trunc, "<");
	struct xml_document* d1 = xml_parse_document(trunc, strlen((char const*)trunc));
	assert_null(d1);
	free(trunc);

	SOURCE(wrong_close, "<root></other>");
	struct xml_document* d2 = xml_parse_document(wrong_close, strlen((char const*)wrong_close));
	assert_null(d2);
	free(wrong_close);

	if (saved_stderr >= 0) {
		dup2(saved_stderr, STDERR_FILENO);
		close(saved_stderr);
	}
}


/**
 * Single self-closing root element (e.g. <a/>). Spec category 1 optional.
 * Parser exposes self-closing tag name including trailing slash (e.g. "a/").
 */
static void test_parse_self_closing_root(void **state) {
	(void)state;
	SOURCE(source, "<a/>");

	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	assert_non_null(document);

	struct xml_node* root = xml_document_root(document);
	assert_non_null(root);
	assert_int_equal(xml_node_children(root), 0);
	/* Self-closing: name is "a/" per current parser behavior */
	assert_true(string_equals(xml_node_name(root), "a/"));

	xml_document_free(document, true);
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
 * Attribute value with spaces in content (upstream #33).
 * Parser uses space-based tokenization, so quoted values containing spaces
 * are split incorrectly. This test fails until quote-aware tokenization
 * is used in attribute parsing.
 *
 * @see docs/issues.md #33
 */
static void test_attribute_value_with_spaces(void **state) {
	(void)state;
	SOURCE(source, "<Root><Node title=\"Hello World\">text</Node></Root>");

	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	assert_non_null(document);

	struct xml_node* root = xml_document_root(document);
	struct xml_node* element = xml_node_child(root, 0);

	assert_non_null(element);
	assert_int_equal(xml_node_attributes(element), 1);
	assert_true(string_equals(xml_node_attribute_name(element, 0), "title"));
	assert_true(string_equals(xml_node_attribute_content(element, 0), "Hello World"));

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
	uint8_t* us_content = xml_node_content_c_string(us);
	assert_non_null(us_content);
	assert_string_equal((char const*)us_content, "testus");
	free(us_content);

	struct xml_node* as = get_node_by_name(root, "as");
	assert_non_null(as);
	uint8_t* as_content = xml_node_content_c_string(as);
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


/**
 * XML comments <!-- ... --> are skipped by the parser (docs/issues.md #21).
 */
static void test_xml_comments_skipped(void **state) {
	(void)state;
	SOURCE(source, ""
		"<!-- before root -->\n"
		"<Root>\n"
		"  <!-- before first -->\n"
		"  <A>one</A>\n"
		"  <!-- between -->\n"
		"  <B>two</B>\n"
		"  <!-- before close -->\n"
		"</Root>\n"
	);
	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	assert_non_null(document);

	struct xml_node* root = xml_document_root(document);
	assert_non_null(root);
	assert_true(string_equals(xml_node_name(root), "Root"));
	assert_int_equal(xml_node_children(root), 2);

	struct xml_node* a = xml_node_child(root, 0);
	struct xml_node* b = xml_node_child(root, 1);
	assert_non_null(a);
	assert_non_null(b);
	assert_true(string_equals(xml_node_name(a), "A"));
	assert_true(string_equals(xml_node_content(a), "one"));
	assert_true(string_equals(xml_node_name(b), "B"));
	assert_true(string_equals(xml_node_content(b), "two"));

	xml_document_free(document, true);
}


/**
 * Comment before root only: minimal comment handling.
 */
static void test_xml_comments_before_root_only(void **state) {
	(void)state;
	SOURCE(source, "<!-- c --><r></r>");
	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	assert_non_null(document);
	struct xml_node* root = xml_document_root(document);
	assert_non_null(root);
	assert_true(string_equals(xml_node_name(root), "r"));
	assert_int_equal(xml_node_children(root), 0);
	xml_document_free(document, true);
}


/**
 * Multiple consecutive comments before root and between nodes.
 */
static void test_xml_comments_consecutive(void **state) {
	(void)state;
	SOURCE(source, "<!-- a --><!-- b --><Root><A></A>\n<!-- mid -->\n<B></B></Root>");
	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	assert_non_null(document);
	struct xml_node* root = xml_document_root(document);
	assert_non_null(root);
	assert_true(string_equals(xml_node_name(root), "Root"));
	assert_int_equal(xml_node_children(root), 2);
	assert_true(string_equals(xml_node_name(xml_node_child(root, 0)), "A"));
	assert_true(string_equals(xml_node_name(xml_node_child(root, 1)), "B"));
	xml_document_free(document, true);
}


/**
 * Empty or minimal comments.
 */
static void test_xml_comments_empty_or_minimal(void **state) {
	(void)state;
	SOURCE(source, "<!----><r></r>");
	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	assert_non_null(document);
	struct xml_node* root = xml_document_root(document);
	assert_non_null(root);
	assert_true(string_equals(xml_node_name(root), "r"));
	xml_document_free(document, true);

	SOURCE(source2, "<!-- --><r></r>");
	document = xml_parse_document(source2, strlen((char const*)source2));
	assert_non_null(document);
	root = xml_document_root(document);
	assert_non_null(root);
	assert_true(string_equals(xml_node_name(root), "r"));
	xml_document_free(document, true);
}


/**
 * Comment between text content and closing tag: comment is skipped, content is what precedes it.
 * (Content stops at first '<'; the comment is skipped when parsing the closing tag.)
 */
static void test_xml_comments_inside_text_before_close(void **state) {
	(void)state;
	SOURCE(source, "<r>hello <!-- ignored --></r>");
	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	assert_non_null(document);
	struct xml_node* root = xml_document_root(document);
	assert_non_null(root);
	assert_true(string_equals(xml_node_name(root), "r"));
	assert_true(string_equals(xml_node_content(root), "hello"));
	assert_int_equal(xml_node_children(root), 0);
	xml_document_free(document, true);
}


/**
 * Comment before a self-closing child.
 * (Self-closing tags yield a tag name that includes the trailing slash, e.g. "empty/".)
 */
static void test_xml_comments_before_self_closing_child(void **state) {
	(void)state;
	SOURCE(source, "<r><!-- comment --><empty/></r>");
	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	assert_non_null(document);
	struct xml_node* root = xml_document_root(document);
	assert_non_null(root);
	assert_true(string_equals(xml_node_name(root), "r"));
	assert_int_equal(xml_node_children(root), 1);
	struct xml_node* child = xml_node_child(root, 0);
	assert_non_null(child);
	assert_true(string_equals(xml_node_name(child), "empty/"));
	xml_document_free(document, true);
}


/**
 * Root with only a comment then closing tag (no children, no text).
 */
static void test_xml_comments_root_only_comment_then_close(void **state) {
	(void)state;
	SOURCE(source, "<r><!-- only --></r>");
	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	assert_non_null(document);
	struct xml_node* root = xml_document_root(document);
	assert_non_null(root);
	assert_true(string_equals(xml_node_name(root), "r"));
	assert_int_equal(xml_node_children(root), 0);
	xml_document_free(document, true);
}


/**
 * Comment spanning multiple lines.
 */
static void test_xml_comments_multiline(void **state) {
	(void)state;
	SOURCE(source, ""
		"<!--\n"
		"  line one\n"
		"  line two\n"
		"--><r>ok</r>"
	);
	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	assert_non_null(document);
	struct xml_node* root = xml_document_root(document);
	assert_non_null(root);
	assert_true(string_equals(xml_node_name(root), "r"));
	assert_true(string_equals(xml_node_content(root), "ok"));
	xml_document_free(document, true);
}


/**
 * xml_node_child with out-of-range index returns NULL.
 */
static void test_node_child_out_of_range(void **state) {
	(void)state;
	SOURCE(source, "<r><a/></r>");

	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	assert_non_null(document);

	struct xml_node* root = xml_document_root(document);
	assert_non_null(root);
	assert_int_equal(xml_node_children(root), 1);

	assert_non_null(xml_node_child(root, 0));
	assert_null(xml_node_child(root, 1));
	assert_null(xml_node_child(root, 99));

	xml_document_free(document, true);
}


/**
 * xml_node_attribute_name / xml_node_attribute_content with out-of-range index return NULL.
 * (_c_string out-of-range is covered in test_attribute_c_string_helpers.)
 */
static void test_node_attribute_out_of_range(void **state) {
	(void)state;
	SOURCE(source, "<r><n foo=\"bar\"/></r>");

	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	assert_non_null(document);

	struct xml_node* root = xml_document_root(document);
	struct xml_node* n = xml_node_child(root, 0);
	assert_non_null(n);
	assert_int_equal(xml_node_attributes(n), 1);

	assert_non_null(xml_node_attribute_name(n, 0));
	assert_non_null(xml_node_attribute_content(n, 0));
	assert_null(xml_node_attribute_name(n, 1));
	assert_null(xml_node_attribute_content(n, 1));
	assert_null(xml_node_attribute_name(n, 99));
	assert_null(xml_node_attribute_content(n, 99));

	xml_document_free(document, true);
}


/**
 * xml_string_length and xml_string_copy in isolation (spec category 5).
 * Parse minimal doc, get node name, assert length and copied content.
 */
static void test_string_length_and_copy(void **state) {
	(void)state;
	SOURCE(source, "<Hello>World</Hello>");

	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	assert_non_null(document);

	struct xml_node* root = xml_document_root(document);
	struct xml_string* name = xml_node_name(root);
	assert_non_null(name);

	size_t len = xml_string_length(name);
	assert_int_equal(len, 5);

	uint8_t* buf = malloc(len + 1);
	assert_non_null(buf);
	xml_string_copy(name, buf, len);
	buf[len] = 0;
	assert_string_equal((char const*)buf, "Hello");
	free(buf);

	xml_document_free(document, true);
}


/**
 * C-string helpers for attributes: xml_node_attribute_name_c_string and
 * xml_node_attribute_content_c_string return 0-terminated copies; out-of-range returns NULL.
 */
static void test_attribute_c_string_helpers(void **state) {
	(void)state;
	SOURCE(source, "<Root><Node a=\"1\" b=\"two\">text</Node></Root>");

	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	assert_non_null(document);

	struct xml_node* root = xml_document_root(document);
	struct xml_node* element = xml_node_child(root, 0);
	assert_non_null(element);
	assert_int_equal(xml_node_attributes(element), 2);

	uint8_t* name0 = xml_node_attribute_name_c_string(element, 0);
	uint8_t* content0 = xml_node_attribute_content_c_string(element, 0);
	assert_non_null(name0);
	assert_non_null(content0);
	assert_string_equal((char const*)name0, "a");
	assert_string_equal((char const*)content0, "1");
	free(name0);
	free(content0);

	uint8_t* name1 = xml_node_attribute_name_c_string(element, 1);
	uint8_t* content1 = xml_node_attribute_content_c_string(element, 1);
	assert_non_null(name1);
	assert_non_null(content1);
	assert_string_equal((char const*)name1, "b");
	assert_string_equal((char const*)content1, "two");
	free(name1);
	free(content1);

	/* Out-of-range index returns NULL */
	assert_null(xml_node_attribute_name_c_string(element, 2));
	assert_null(xml_node_attribute_content_c_string(element, 2));

	xml_document_free(document, true);
}


/**
 * xml_string_equals: same content returns true, different content or NULL returns false.
 */
static void test_xml_string_equals(void **state) {
	(void)state;
	SOURCE(source, "<r><a>x</a><b>y</b><c>x</c></r>");

	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	assert_non_null(document);

	struct xml_node* root = xml_document_root(document);
	struct xml_node* a = xml_node_child(root, 0);
	struct xml_node* b = xml_node_child(root, 1);
	struct xml_node* c = xml_node_child(root, 2);
	assert_non_null(a);
	assert_non_null(b);
	assert_non_null(c);

	struct xml_string* content_a = xml_node_content(a);
	struct xml_string* content_b = xml_node_content(b);
	struct xml_string* content_c = xml_node_content(c);
	assert_true(xml_string_equals(content_a, content_c));
	assert_false(xml_string_equals(content_a, content_b));
	assert_false(xml_string_equals(content_a, NULL));
	assert_false(xml_string_equals(NULL, content_a));

	xml_document_free(document, true);
}


/**
 * xml_string_equals_cstr: compare xml_string to C string; NULL string returns false; NULL cstr treated as empty.
 */
static void test_xml_string_equals_cstr(void **state) {
	(void)state;
	SOURCE(source, "<r><a>hello</a></r>");

	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	assert_non_null(document);

	struct xml_node* root = xml_document_root(document);
	struct xml_node* a = xml_node_child(root, 0);
	assert_non_null(a);

	struct xml_string* content_hello = xml_node_content(a);

	assert_true(xml_string_equals_cstr(content_hello, (uint8_t const*)"hello"));
	assert_false(xml_string_equals_cstr(content_hello, (uint8_t const*)"world"));
	assert_false(xml_string_equals_cstr(content_hello, (uint8_t const*)"hell"));
	assert_false(xml_string_equals_cstr(NULL, (uint8_t const*)"x"));
	/* NULL cstr treated as empty: compare only when we have a length-0 string; node content for empty element may be NULL so we skip that here */

	xml_document_free(document, true);
}


/**
 * Element with no text content: xml_node_content is NULL, xml_node_content_c_string returns NULL.
 */
static void test_node_content_empty_element(void **state) {
	(void)state;
	SOURCE(source, "<r><empty></empty><self/><child><inner/></child></r>");

	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	assert_non_null(document);

	struct xml_node* root = xml_document_root(document);
	struct xml_node* empty = xml_node_child(root, 0);
	struct xml_node* self = xml_node_child(root, 1);
	struct xml_node* child = xml_node_child(root, 2);

	assert_null(xml_node_content(empty));
	assert_null(xml_node_content_c_string(empty));

	assert_null(xml_node_content(self));
	assert_null(xml_node_content_c_string(self));

	assert_null(xml_node_content(child));
	assert_null(xml_node_content_c_string(child));

	xml_document_free(document, true);
}


/**
 * xml_string_copy with buffer shorter than string: copies only length bytes (no overrun).
 */
static void test_xml_string_copy_partial(void **state) {
	(void)state;
	SOURCE(source, "<r>Hello</r>");

	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	assert_non_null(document);

	struct xml_string* content = xml_node_content(xml_document_root(document));
	assert_non_null(content);
	assert_int_equal(xml_string_length(content), 5);

	uint8_t buf[3];
	memset(buf, 0xAA, sizeof(buf));
	xml_string_copy(content, buf, 2);
	buf[2] = 0;
	assert_int_equal((unsigned char)buf[0], (unsigned char)'H');
	assert_int_equal((unsigned char)buf[1], (unsigned char)'e');
	assert_int_equal((unsigned char)buf[2], 0);

	xml_document_free(document, true);
}


/**
 * xml_string_copy with length 0: no write (documented: at most length bytes).
 */
static void test_xml_string_copy_zero_length(void **state) {
	(void)state;
	SOURCE(source, "<r>x</r>");

	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	assert_non_null(document);

	struct xml_string* content = xml_node_content(xml_document_root(document));
	uint8_t buf[4] = { 0xBB, 0xBB, 0xBB, 0xBB };
	xml_string_copy(content, buf, 0);
	assert_int_equal(buf[0], 0xBB);
	assert_int_equal(buf[1], 0xBB);

	xml_document_free(document, true);
}


/**
 * Document with only whitespace (no root tag) must return NULL.
 */
static void test_parse_whitespace_only_returns_null(void **state) {
	(void)state;
	SOURCE(source, "   \n\t  \r\n ");

	int saved_stderr = dup(STDERR_FILENO);
	if (saved_stderr >= 0) {
		int devnull = open("/dev/null", O_WRONLY);
		if (devnull >= 0) {
			dup2(devnull, STDERR_FILENO);
			close(devnull);
		}
	}

	struct xml_document* doc = xml_parse_document(source, strlen((char const*)source));
	assert_null(doc);

	if (saved_stderr >= 0) {
		dup2(saved_stderr, STDERR_FILENO);
		close(saved_stderr);
	}
	free(source);
}


/**
 * Malformed XML: unclosed attribute quote must yield NULL.
 */
static void test_parse_malformed_unclosed_quote(void **state) {
	(void)state;
	SOURCE(source, "<root a=\"unclosed");

	int saved_stderr = dup(STDERR_FILENO);
	if (saved_stderr >= 0) {
		int devnull = open("/dev/null", O_WRONLY);
		if (devnull >= 0) {
			dup2(devnull, STDERR_FILENO);
			close(devnull);
		}
	}

	struct xml_document* doc = xml_parse_document(source, strlen((char const*)source));
	assert_null(doc);

	if (saved_stderr >= 0) {
		dup2(saved_stderr, STDERR_FILENO);
		close(saved_stderr);
	}
	free(source);
}


/**
 * Attribute with empty value: parsed and content has length 0.
 */
static void test_attribute_empty_value(void **state) {
	(void)state;
	SOURCE(source, "<r><n empty=\"\" a=\"b\"/></r>");

	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	assert_non_null(document);

	struct xml_node* root = xml_document_root(document);
	struct xml_node* n = xml_node_child(root, 0);
	assert_int_equal(xml_node_attributes(n), 2);

	struct xml_string* empty_attr = xml_node_attribute_content(n, 0);
	assert_non_null(empty_attr);
	assert_int_equal(xml_string_length(empty_attr), 0);
	assert_true(xml_string_equals_cstr(empty_attr, (uint8_t const*)""));

	struct xml_string* second = xml_node_attribute_content(n, 1);
	assert_true(xml_string_equals_cstr(second, (uint8_t const*)"b"));

	xml_document_free(document, true);
}


/**
 * xml_easy_child returns NULL when two children have the same name (path ambiguous).
 */
static void test_easy_child_returns_null_when_duplicate_children(void **state) {
	(void)state;
	SOURCE(source, "<r><Foo>a</Foo><Foo>b</Foo></r>");

	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	assert_non_null(document);

	struct xml_node* root = xml_document_root(document);
	assert_int_equal(xml_node_children(root), 2);

	struct xml_node* ambiguous = xml_easy_child(root, (uint8_t const*)"Foo", (uint8_t const*)0);
	assert_null(ambiguous);

	xml_document_free(document, true);
}


/**
 * Document that is only a comment (no root element) must return NULL.
 */
static void test_parse_only_comment_returns_null(void **state) {
	(void)state;
	SOURCE(source, "<!-- only a comment -->");

	int saved_stderr = dup(STDERR_FILENO);
	if (saved_stderr >= 0) {
		int devnull = open("/dev/null", O_WRONLY);
		if (devnull >= 0) {
			dup2(devnull, STDERR_FILENO);
			close(devnull);
		}
	}

	struct xml_document* doc = xml_parse_document(source, strlen((char const*)source));
	assert_null(doc);

	if (saved_stderr >= 0) {
		dup2(saved_stderr, STDERR_FILENO);
		close(saved_stderr);
	}
	free(source);
}


/**
 * Document that is only a processing instruction (no root element) must return NULL.
 */
static void test_parse_only_pi_returns_null(void **state) {
	(void)state;
	SOURCE(source, "<?xml version=\"1.0\"?>");

	int saved_stderr = dup(STDERR_FILENO);
	if (saved_stderr >= 0) {
		int devnull = open("/dev/null", O_WRONLY);
		if (devnull >= 0) {
			dup2(devnull, STDERR_FILENO);
			close(devnull);
		}
	}

	struct xml_document* doc = xml_parse_document(source, strlen((char const*)source));
	assert_null(doc);

	if (saved_stderr >= 0) {
		dup2(saved_stderr, STDERR_FILENO);
		close(saved_stderr);
	}
	free(source);
}


/**
 * xml_string_equals_cstr with NULL cstr: API treats NULL as empty; empty string equals NULL cstr.
 */
static void test_xml_string_equals_cstr_null_treated_as_empty(void **state) {
	(void)state;
	SOURCE(source, "<r a=\"\"/>");

	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	assert_non_null(document);

	struct xml_node* root = xml_document_root(document);
	struct xml_string* attr_content = xml_node_attribute_content(root, 0);
	assert_non_null(attr_content);
	assert_int_equal(xml_string_length(attr_content), 0);

	assert_true(xml_string_equals_cstr(attr_content, NULL));
	assert_true(xml_string_equals_cstr(attr_content, (uint8_t const*)""));

	xml_document_free(document, true);
}


/**
 * Mixed content: text before and after child is concatenated as node content.
 */
static void test_mixed_content_text_before_child(void **state) {
	(void)state;
	SOURCE(source, "<r>before<child/>after</r>");

	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	assert_non_null(document);

	struct xml_node* root = xml_document_root(document);
	assert_true(string_equals(xml_node_content(root), "beforeafter"));
	assert_int_equal(xml_node_children(root), 1);

	struct xml_node* ch = xml_node_child(root, 0);
	assert_non_null(ch);
	assert_true(string_equals(xml_node_name(ch), "child/"));

	xml_document_free(document, true);
}


/**
 * Element with several attributes: count and last name/value correct.
 */
static void test_many_attributes(void **state) {
	(void)state;
	SOURCE(source, "<r><n a1=\"v1\" a2=\"v2\" a3=\"v3\" a4=\"v4\" a5=\"v5\"/></r>");

	struct xml_document* document = xml_parse_document(source, strlen((char const*)source));
	assert_non_null(document);

	struct xml_node* root = xml_document_root(document);
	struct xml_node* n = xml_node_child(root, 0);
	assert_int_equal(xml_node_attributes(n), 5);

	assert_true(string_equals(xml_node_attribute_name(n, 0), "a1"));
	assert_true(string_equals(xml_node_attribute_content(n, 0), "v1"));
	assert_true(string_equals(xml_node_attribute_name(n, 4), "a5"));
	assert_true(string_equals(xml_node_attribute_content(n, 4), "v5"));

	xml_document_free(document, true);
}


/**
 * Malformed: stray closing tag (no opening) must yield NULL.
 */
static void test_parse_malformed_stray_close_returns_null(void **state) {
	(void)state;
	SOURCE(source, "</orphan>");

	int saved_stderr = dup(STDERR_FILENO);
	if (saved_stderr >= 0) {
		int devnull = open("/dev/null", O_WRONLY);
		if (devnull >= 0) {
			dup2(devnull, STDERR_FILENO);
			close(devnull);
		}
	}

	struct xml_document* doc = xml_parse_document(source, strlen((char const*)source));
	assert_null(doc);

	if (saved_stderr >= 0) {
		dup2(saved_stderr, STDERR_FILENO);
		close(saved_stderr);
	}
	free(source);
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
	cmocka_unit_test(test_parse_empty_buffer),
	cmocka_unit_test(test_parse_malformed_returns_null),
	cmocka_unit_test(test_parse_self_closing_root),
	cmocka_unit_test(test_parse_whitespace_only_returns_null),
	cmocka_unit_test(test_parse_malformed_unclosed_quote),
	cmocka_unit_test(test_parse_only_comment_returns_null),
	cmocka_unit_test(test_parse_only_pi_returns_null),
	cmocka_unit_test(test_parse_malformed_stray_close_returns_null),
	cmocka_unit_test(test_attributes_in_memory_0),
	cmocka_unit_test(test_attributes_in_memory_1),
	cmocka_unit_test(test_attributes_in_memory_2),
	cmocka_unit_test(test_attribute_value_with_spaces),
	cmocka_unit_test(test_attributes_from_file_0),
	cmocka_unit_test(test_attributes_from_file_1),
	cmocka_unit_test(test_attributes_from_file_2),
	cmocka_unit_test(test_attribute_empty_value),
	cmocka_unit_test(test_many_attributes),
	cmocka_unit_test(test_tiled_svg_style_multiline_opening_tag),
	cmocka_unit_test(test_find_node_by_tag_name),
	cmocka_unit_test(test_easy_child_returns_null_when_duplicate_children),
	cmocka_unit_test(test_mixed_content_text_before_child),
	cmocka_unit_test(test_node_child_out_of_range),
	cmocka_unit_test(test_node_attribute_out_of_range),
	cmocka_unit_test(test_node_content_empty_element),
	cmocka_unit_test(test_string_length_and_copy),
	cmocka_unit_test(test_xml_string_copy_partial),
	cmocka_unit_test(test_xml_string_copy_zero_length),
	cmocka_unit_test(test_xml_string_equals_cstr_null_treated_as_empty),
	cmocka_unit_test(test_parse_error_at_end_of_buffer),
	cmocka_unit_test(test_realloc_failure_no_leak),
	cmocka_unit_test(test_xml_comments_skipped),
	cmocka_unit_test(test_xml_comments_before_root_only),
	cmocka_unit_test(test_xml_comments_consecutive),
	cmocka_unit_test(test_xml_comments_empty_or_minimal),
	cmocka_unit_test(test_xml_comments_inside_text_before_close),
	cmocka_unit_test(test_xml_comments_before_self_closing_child),
	cmocka_unit_test(test_xml_comments_root_only_comment_then_close),
	cmocka_unit_test(test_xml_comments_multiline),
	cmocka_unit_test(test_attribute_c_string_helpers),
	cmocka_unit_test(test_xml_string_equals),
	cmocka_unit_test(test_xml_string_equals_cstr),
};

void get_unit_c_tests(const struct CMUnitTest** out_tests, size_t* out_count) {
	*out_tests = tests;
	*out_count = sizeof(tests) / sizeof(tests[0]);
}
