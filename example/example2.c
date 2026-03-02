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
 *
 * Second example: parses a single XML document that exercises most features
 * added in this fork (encoding, DOCTYPE, PIs, comments, CDATA, entities,
 * namespaces, unique attributes, Name production, c_string helpers,
 * xml_easy_child, xml_document_buffer_length, xml_string_equals_cstr).
 * Each feature is pointed out with the relevant XML and the parsed result.
 */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xml.h>

/* One document that showcases many features. The parser will:
 * - Accept XML declaration with encoding="UTF-8"
 * - Skip DOCTYPE and the optional internal subset
 * - Skip the processing instruction <?xml-stylesheet ...?>
 * - Skip the comment <!-- ... -->
 * - Expose xmlns / xmlns:app as attributes (namespace feature)
 * - Parse multiple attributes with unique names, including values with spaces
 * - Expose CDATA content as literal text (no entity interpretation inside)
 * - Expand &amp; &#65; etc. in element text and attribute values
 * - Accept tag names that match the Name production (_catalog, app:item, etc.)
 */
static char const doc[] =
	"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
	"<!DOCTYPE catalog SYSTEM \"catalog.dtd\" [\n"
	"  <!ELEMENT catalog (section+)>\n"
	"]>\n"
	"<?xml-stylesheet type=\"text/css\" href=\"style.css\"?>\n"
	"<!-- Demo document for xml.c feature set -->\n"
	"<_catalog xmlns=\"http://example.com/default\" xmlns:app=\"http://example.com/app\">\n"
	"  <section id=\"s1\" title=\"First Section\" count=\"2\">\n"
	"    <app:item status=\"ready\">Text with &amp; and &#65; and &lt;tags&gt;</app:item>\n"
	"    <raw><![CDATA[Literal <b>markup</b> and &amp; here are not interpreted]]></raw>\n"
	"  </section>\n"
	"</_catalog>\n";


static void print_sep(char const* title) {
	printf("\n--- %s ---\n", title);
}


int main(void) {
	size_t doc_len = strlen(doc);
	uint8_t* buffer = (uint8_t*)doc;   /* parser keeps reference; do not free until document freed */

	struct xml_document* document = xml_parse_document(buffer, doc_len);
	if (!document) {
		printf("Error: Could not parse document.\n");
		return EXIT_FAILURE;
	}

	struct xml_node* root = xml_document_root(document);
	if (!root) {
		xml_document_free(document, false);
		return EXIT_FAILURE;
	}

	struct xml_node* section = NULL;
	struct xml_node* item = NULL;

	/* Feature: xml_document_buffer_length */
	print_sep("xml_document_buffer_length");
	printf("Document buffer length (bytes): %zu\n", xml_document_buffer_length(document));
	printf("(Relevant XML: whole buffer; useful for validation vs file size.)\n");

	/* Feature: XML declaration with encoding UTF-8 (parser accepted the document) */
	print_sep("XML declaration (encoding UTF-8)");
	printf("Document parsed successfully.\n");
	printf("(Relevant XML: <?xml version=\"1.0\" encoding=\"UTF-8\"?> — non-UTF-8 encoding would be rejected.)\n");

	/* Feature: DOCTYPE skipped */
	print_sep("DOCTYPE (skipped)");
	printf("Root element is _catalog; DOCTYPE was skipped.\n");
	printf("(Relevant XML: <!DOCTYPE catalog SYSTEM \"catalog.dtd\" [ ... ]> )\n");

	/* Feature: Processing instruction and comment */
	print_sep("Processing instruction and comment");
	printf("PI and comment before root are skipped; root is the first element.\n");
	printf("(Relevant XML: <?xml-stylesheet ...?> and <!-- ... -->)\n");

	/* Feature: Tag names (Name production) and xml_node_name_c_string */
	print_sep("Tag names (Name production) and xml_node_name_c_string");
	uint8_t* root_name = xml_node_name_c_string(root);
	if (root_name) {
		printf("Root tag name (c_string): %s\n", root_name);
		free(root_name);
	}
	printf("(Relevant XML: <_catalog> — names must start with letter, _, or :.)\n");

	/* Feature: Namespaces (xmlns / xmlns:prefix as attributes) */
	print_sep("Namespaces (xmlns as attributes)");
	size_t n_attr = xml_node_attributes(root);
	printf("Root has %zu attribute(s).\n", n_attr);
	for (size_t i = 0; i < n_attr; i++) {
		uint8_t* aname = xml_node_attribute_name_c_string(root, i);
		uint8_t* aval  = xml_node_attribute_content_c_string(root, i);
		if (aname && aval) {
			printf("  %s = \"%s\"\n", aname, aval);
			free(aname);
			free(aval);
		}
	}
	printf("(Relevant XML: xmlns=\"...\" xmlns:app=\"...\" — exposed as attributes.)\n");

	/* Feature: xml_easy_child */
	print_sep("xml_easy_child (path lookup)");
	section = xml_easy_child(root, (uint8_t const*)"section", NULL);
	if (section) {
		uint8_t* sec_name = xml_node_name_c_string(section);
		if (sec_name) {
			printf("Found path root -> section: tag \"%s\"\n", sec_name);
			free(sec_name);
		}
	} else {
		printf("Path root -> section not found.\n");
	}
	printf("(Relevant XML: <section id=\"s1\" ...> under <_catalog>.)\n");

	/* Feature: Unique attributes and attributes with spaces */
	print_sep("Unique attributes and attributes with spaces");
	if (section) {
		size_t sa = xml_node_attributes(section);
		printf("Section has %zu attribute(s).\n", sa);
		for (size_t i = 0; i < sa; i++) {
			uint8_t* aname = xml_node_attribute_name_c_string(section, i);
			uint8_t* aval  = xml_node_attribute_content_c_string(section, i);
			if (aname && aval) {
				printf("  %s = \"%s\"\n", aname, aval);
				free(aname);
				free(aval);
			}
		}
	}
	printf("(Relevant XML: id=\"s1\" title=\"First Section\" count=\"2\" — unique names; title has a space.)\n");

	/* Feature: Entity and character reference expansion */
	print_sep("Entity and character reference expansion");
	item = xml_easy_child(root, (uint8_t const*)"section", (uint8_t const*)"app:item", NULL);
	if (item) {
		uint8_t* content = xml_node_content_c_string(item);
		if (content) {
			printf("app:item content (after expansion): \"%s\"\n", content);
			free(content);
		}
		/* Attribute with entities */
		uint8_t* status = xml_node_attribute_content_c_string(item, 0);
		if (status) {
			printf("status attribute: \"%s\"\n", status);
			free(status);
		}
	}
	printf("(Relevant XML: &amp; &#65; &lt; &gt; in text and attributes — expanded to & A < >.)\n");

	/* Feature: xml_string_equals_cstr */
	print_sep("xml_string_equals_cstr");
	struct xml_string* item_name = xml_node_name(item ? item : section);
	if (item_name && xml_string_equals_cstr(item_name, (uint8_t const*)"app:item")) {
		printf("Node name equals \"app:item\" (xml_string_equals_cstr).\n");
	} else {
		printf("Node name comparison: not \"app:item\" or node missing.\n");
	}
	printf("(Relevant API: compare xml_string to C string; NULL string treated as empty.)\n");

	/* Feature: CDATA */
	print_sep("CDATA sections");
	struct xml_node* raw = xml_easy_child(root, (uint8_t const*)"section", (uint8_t const*)"raw", NULL);
	if (raw) {
		uint8_t* raw_content = xml_node_content_c_string(raw);
		if (raw_content) {
			printf("raw content (CDATA, literal): \"%s\"\n", raw_content);
			free(raw_content);
		}
	}
	printf("(Relevant XML: <![CDATA[Literal <b>markup</b> and &amp; ...]]> — no markup/entity interpretation.)\n");

	printf("\n--- Done ---\n");
	xml_document_free(document, false);
	return EXIT_SUCCESS;
}
