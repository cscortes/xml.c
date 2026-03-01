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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
 * Tests the eas functionality
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

	struct xml_node* test_a = xml_easy_child(root, "This", "Is", "A", "Test", 0);
	assert_non_null(test_a);
	assert_true(string_equals(xml_node_content(test_a), "Content A"));

	struct xml_node* test_b = xml_easy_child(root, "This", "Is", "B", "Test", 0);
	assert_non_null(test_b);
	assert_true(string_equals(xml_node_content(test_b), "Content B"));

	struct xml_node* test_c = xml_easy_child(root, "This", "Is", "C", "Test", 0);
	assert_null(test_c);

	struct xml_node* must_be_null = xml_easy_child(root, "Child");
	assert_null(must_be_null);

	uint8_t* name_is = xml_easy_name(xml_easy_child(root, "This", "Is", 0));
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
		xml_document_root(document), "Element", "With", 0
	);
	assert_non_null(element);
	assert_true(string_equals(xml_node_content(element), "Child"));

	xml_document_free(document, true);
	#undef FILE_NAME
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


static const struct CMUnitTest tests[] = {
	cmocka_unit_test(test_xml_parse_document_0),
	cmocka_unit_test(test_xml_parse_document_1),
	cmocka_unit_test(test_xml_parse_document_2),
	cmocka_unit_test(test_xml_parse_document_3),
	cmocka_unit_test(test_attributes_in_memory_0),
	cmocka_unit_test(test_attributes_in_memory_1),
	cmocka_unit_test(test_attributes_in_memory_2),
	cmocka_unit_test(test_attributes_from_file_0),
	cmocka_unit_test(test_attributes_from_file_1),
	cmocka_unit_test(test_attributes_from_file_2),
	cmocka_unit_test(test_find_node_by_tag_name),
};

void get_unit_c_tests(const struct CMUnitTest** out_tests, size_t* out_count) {
	*out_tests = tests;
	*out_count = sizeof(tests) / sizeof(tests[0]);
}
