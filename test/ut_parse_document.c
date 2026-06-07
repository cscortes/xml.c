/*
 * Feature unit tests for xml_parse_document — cases P-01 through P-69.
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

/* Linker-wrapped realloc provided by wrap_realloc.c (--wrap=realloc). */
extern void *__wrap_realloc(void *ptr, size_t size);


/* P-01: Minimal self-closing root. */
static void test_p01_minimal_self_closing(void **state) {
	(void)state;
	SOURCE(s, "<root/>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	xml_document_free(doc, true);
}

/* P-02: Minimal open/close root. */
static void test_p02_minimal_open_close(void **state) {
	(void)state;
	SOURCE(s, "<root></root>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	xml_document_free(doc, true);
}

/* P-03: Root with text content. */
static void test_p03_root_with_text(void **state) {
	(void)state;
	SOURCE(s, "<r>hello</r>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	struct xml_node *root = xml_document_root(doc);
	assert_true(xml_str_eq(xml_node_content(root), "hello"));
	xml_document_free(doc, true);
}

/* P-04: Root with child element. */
static void test_p04_root_with_child(void **state) {
	(void)state;
	SOURCE(s, "<r><c/></r>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	struct xml_node *root = xml_document_root(doc);
	assert_int_equal(xml_node_children(root), 1);
	xml_document_free(doc, true);
}

/* P-05: Root with attribute. */
static void test_p05_root_with_attribute(void **state) {
	(void)state;
	SOURCE(s, "<r a=\"v\"/>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	struct xml_node *root = xml_document_root(doc);
	assert_int_equal(xml_node_attributes(root), 1);
	xml_document_free(doc, true);
}

/* P-06: XML declaration present. */
static void test_p06_xml_declaration(void **state) {
	(void)state;
	SOURCE(s, "<?xml version=\"1.0\"?><r/>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	xml_document_free(doc, true);
}

/* P-07: XML declaration + encoding UTF-8. */
static void test_p07_xml_decl_utf8(void **state) {
	(void)state;
	SOURCE(s, "<?xml version=\"1.0\" encoding=\"UTF-8\"?><r/>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	xml_document_free(doc, true);
}

/* P-08: XML declaration + non-UTF-8 encoding — rejected. */
static void test_p08_xml_decl_non_utf8(void **state) {
	(void)state;
	SOURCE(s, "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?><r/>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_null(doc);
	free(s);
}

/* P-09: Comment before root. */
static void test_p09_comment_before_root(void **state) {
	(void)state;
	SOURCE(s, "<!-- c --><r></r>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	struct xml_node *root = xml_document_root(doc);
	assert_true(xml_str_eq(xml_node_name(root), "r"));
	xml_document_free(doc, true);
}

/* P-10: Multiple comments before root. */
static void test_p10_multiple_comments(void **state) {
	(void)state;
	SOURCE(s, "<!-- a --><!-- b --><r/>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	xml_document_free(doc, true);
}

/* P-11: PI before root. */
static void test_p11_pi_before_root(void **state) {
	(void)state;
	SOURCE(s, "<?pi data?><r/>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	xml_document_free(doc, true);
}

/* P-12: Multiple PIs before root. */
static void test_p12_multiple_pis(void **state) {
	(void)state;
	SOURCE(s, "<?a?><?b?><r/>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	xml_document_free(doc, true);
}

/* P-13: PI and comment mix before root. */
static void test_p13_pi_comment_mix(void **state) {
	(void)state;
	SOURCE(s, "<?pi?> <!-- c --> <r/>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	xml_document_free(doc, true);
}

/* P-14: DOCTYPE minimal. */
static void test_p14_doctype_minimal(void **state) {
	(void)state;
	SOURCE(s, "<!DOCTYPE r><r/>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	xml_document_free(doc, true);
}

/* P-15: DOCTYPE with empty internal subset. */
static void test_p15_doctype_empty_subset(void **state) {
	(void)state;
	SOURCE(s, "<!DOCTYPE r []><r/>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	xml_document_free(doc, true);
}

/* P-16: DOCTYPE with SYSTEM id (skipped). */
static void test_p16_doctype_system_id(void **state) {
	(void)state;
	SOURCE(s, "<!DOCTYPE r SYSTEM \"x.dtd\"><r/>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	xml_document_free(doc, true);
}

/* P-17: Whitespace only — no root. */
static void test_p17_whitespace_only(void **state) {
	(void)state;
	SOURCE(s, "   ");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_null(doc);
	free(s);
}

/* P-18: Empty buffer (length=0). */
static void test_p18_empty_buffer(void **state) {
	(void)state;
	uint8_t buf[1] = {0};
	struct xml_document *doc = xml_parse_document(buf, 0);
	assert_null(doc);
}

/* P-19: Only a comment, no root. */
static void test_p19_comment_no_root(void **state) {
	(void)state;
	SOURCE(s, "<!-- no root -->");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_null(doc);
	free(s);
}

/* P-20: Only a PI, no root. */
static void test_p20_pi_no_root(void **state) {
	(void)state;
	SOURCE(s, "<?pi?>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_null(doc);
	free(s);
}

/* P-21: Unclosed tag. */
static void test_p21_unclosed_tag(void **state) {
	(void)state;
	SOURCE(s, "<root>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_null(doc);
	free(s);
}

/* P-22: Mismatched close tag. */
static void test_p22_mismatched_close(void **state) {
	(void)state;
	SOURCE(s, "<root></wrong>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_null(doc);
	free(s);
}

/* P-23: Stray close tag. */
static void test_p23_stray_close(void **state) {
	(void)state;
	SOURCE(s, "</root>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_null(doc);
	free(s);
}

/* P-24: Unclosed attribute quote. */
static void test_p24_unclosed_attr_quote(void **state) {
	(void)state;
	SOURCE(s, "<root a=\"v>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_null(doc);
	free(s);
}

/* P-25: Tag name starts with digit. */
static void test_p25_digit_start_tag(void **state) {
	(void)state;
	SOURCE(s, "<1root/>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_null(doc);
	free(s);
}

/* P-26: Unescaped & in content. */
static void test_p26_unescaped_amp_content(void **state) {
	(void)state;
	SOURCE(s, "<r>foo & bar</r>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_null(doc);
	free(s);
}

/* P-27: Unescaped & at end of content. */
static void test_p27_unescaped_amp_end(void **state) {
	(void)state;
	SOURCE(s, "<r>foo &</r>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_null(doc);
	free(s);
}

/* P-28: Unescaped & in attribute value. */
static void test_p28_unescaped_amp_attr(void **state) {
	(void)state;
	SOURCE(s, "<r a=\"a & b\"/>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_null(doc);
	free(s);
}

/* P-29: Unknown entity reference. */
static void test_p29_unknown_entity(void **state) {
	(void)state;
	SOURCE(s, "<r>&unknown;</r>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_null(doc);
	free(s);
}

/* P-30: Duplicate attribute on same element. */
static void test_p30_duplicate_attribute(void **state) {
	(void)state;
	SOURCE(s, "<r a=\"1\" a=\"2\"/>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_null(doc);
	free(s);
}

/* P-31: length shorter than actual content — truncated parse fails. */
static void test_p31_length_shorter(void **state) {
	(void)state;
	SOURCE(s, "<root/>");
	struct xml_document *doc = xml_parse_document(s, 3);
	assert_null(doc);
	free(s);
}

/* P-32: length exactly matches content. */
static void test_p32_length_exact(void **state) {
	(void)state;
	SOURCE(s, "<r/>");
	struct xml_document *doc = xml_parse_document(s, 4);
	assert_non_null(doc);
	xml_document_free(doc, true);
}

/* P-33: length larger than actual content (trailing zeros). */
static void test_p33_length_larger(void **state) {
	(void)state;
	uint8_t buf[20];
	memset(buf, 0, sizeof(buf));
	memcpy(buf, "<r/>", 4);
	struct xml_document *doc = xml_parse_document(buf, 20);
	assert_non_null(doc);
	xml_document_free(doc, false);
}

/* P-34: Deep nesting — navigable. */
static void test_p34_deep_nesting(void **state) {
	(void)state;
	SOURCE(s, "<a><b><c><d>v</d></c></b></a>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	struct xml_node *a = xml_document_root(doc);
	struct xml_node *b = xml_node_child(a, 0);
	struct xml_node *c = xml_node_child(b, 0);
	struct xml_node *d = xml_node_child(c, 0);
	assert_non_null(d);
	assert_true(xml_str_eq(xml_node_content(d), "v"));
	xml_document_free(doc, true);
}

/* P-35: Wide tree — four siblings. */
static void test_p35_wide_tree(void **state) {
	(void)state;
	SOURCE(s, "<r><a/><b/><c/><d/></r>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	struct xml_node *root = xml_document_root(doc);
	assert_int_equal(xml_node_children(root), 4);
	xml_document_free(doc, true);
}

/* P-36: Multiline opening tag (Tiled/SVG style). */
static void test_p36_multiline_tag(void **state) {
	(void)state;
	SOURCE(s, "<r\n  a=\"1\"\n  b=\"2\"\n/>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	struct xml_node *root = xml_document_root(doc);
	assert_int_equal(xml_node_attributes(root), 2);
	xml_document_free(doc, true);
}

/* P-37: Mixed content (text + child elements). */
static void test_p37_mixed_content(void **state) {
	(void)state;
	SOURCE(s, "<r>text<c/>more</r>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	xml_document_free(doc, true);
}

/* P-38: &amp; entity in content. */
static void test_p38_entity_amp(void **state) {
	(void)state;
	SOURCE(s, "<r>&amp;</r>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	assert_true(xml_str_eq(xml_node_content(xml_document_root(doc)), "&"));
	xml_document_free(doc, true);
}

/* P-39: &lt; entity in content. */
static void test_p39_entity_lt(void **state) {
	(void)state;
	SOURCE(s, "<r>&lt;</r>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	assert_true(xml_str_eq(xml_node_content(xml_document_root(doc)), "<"));
	xml_document_free(doc, true);
}

/* P-40: &gt; entity in content. */
static void test_p40_entity_gt(void **state) {
	(void)state;
	SOURCE(s, "<r>&gt;</r>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	assert_true(xml_str_eq(xml_node_content(xml_document_root(doc)), ">"));
	xml_document_free(doc, true);
}

/* P-41: &quot; entity in content. */
static void test_p41_entity_quot(void **state) {
	(void)state;
	SOURCE(s, "<r>&quot;</r>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	assert_true(xml_str_eq(xml_node_content(xml_document_root(doc)), "\""));
	xml_document_free(doc, true);
}

/* P-42: &apos; entity in content. */
static void test_p42_entity_apos(void **state) {
	(void)state;
	SOURCE(s, "<r>&apos;</r>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	assert_true(xml_str_eq(xml_node_content(xml_document_root(doc)), "'"));
	xml_document_free(doc, true);
}

/* P-43: Mixed entities in content. */
static void test_p43_entity_mixed(void **state) {
	(void)state;
	SOURCE(s, "<r>&amp;&lt;&gt;</r>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	assert_true(xml_str_eq(xml_node_content(xml_document_root(doc)), "&<>"));
	xml_document_free(doc, true);
}

/* P-44: Decimal character reference in content. */
static void test_p44_char_ref_decimal(void **state) {
	(void)state;
	SOURCE(s, "<r>&#65;</r>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	assert_true(xml_str_eq(xml_node_content(xml_document_root(doc)), "A"));
	xml_document_free(doc, true);
}

/* P-45: Hex character reference in content. */
static void test_p45_char_ref_hex(void **state) {
	(void)state;
	SOURCE(s, "<r>&#x41;</r>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	assert_true(xml_str_eq(xml_node_content(xml_document_root(doc)), "A"));
	xml_document_free(doc, true);
}

/* P-46: Uppercase X in hex char ref — library accepts &#X41; and yields "A". */
static void test_p46_char_ref_uppercase_x(void **state) {
	(void)state;
	SOURCE(s, "<r>&#X41;</r>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	assert_true(xml_str_eq(xml_node_content(xml_document_root(doc)), "A"));
	xml_document_free(doc, true);
}

/* P-47: Entity in attribute value. */
static void test_p47_entity_in_attr(void **state) {
	(void)state;
	SOURCE(s, "<r a=\"&amp;\"/>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	struct xml_node *root = xml_document_root(doc);
	assert_true(xml_str_eq(xml_node_attribute_content(root, 0), "&"));
	xml_document_free(doc, true);
}

/* P-48: Decimal char ref in attribute value. */
static void test_p48_char_ref_decimal_attr(void **state) {
	(void)state;
	SOURCE(s, "<r a=\"&#65;\"/>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	struct xml_node *root = xml_document_root(doc);
	assert_true(xml_str_eq(xml_node_attribute_content(root, 0), "A"));
	xml_document_free(doc, true);
}

/* P-49: Hex char ref in attribute value. */
static void test_p49_char_ref_hex_attr(void **state) {
	(void)state;
	SOURCE(s, "<r a=\"&#x41;\"/>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	struct xml_node *root = xml_document_root(doc);
	assert_true(xml_str_eq(xml_node_attribute_content(root, 0), "A"));
	xml_document_free(doc, true);
}

/* P-50: CDATA section simple. */
static void test_p50_cdata_simple(void **state) {
	(void)state;
	SOURCE(s, "<r><![CDATA[hello]]></r>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	assert_true(xml_str_eq(xml_node_content(xml_document_root(doc)), "hello"));
	xml_document_free(doc, true);
}

/* P-51: CDATA with angle brackets — not parsed as markup. */
static void test_p51_cdata_angle_brackets(void **state) {
	(void)state;
	SOURCE(s, "<r><![CDATA[<not-a-tag>]]></r>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	assert_true(xml_str_eq(xml_node_content(xml_document_root(doc)), "<not-a-tag>"));
	xml_document_free(doc, true);
}

/* P-52: CDATA with ampersand — not entity-expanded. */
static void test_p52_cdata_ampersand_literal(void **state) {
	(void)state;
	SOURCE(s, "<r><![CDATA[&amp;]]></r>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	assert_true(xml_str_eq(xml_node_content(xml_document_root(doc)), "&amp;"));
	xml_document_free(doc, true);
}

/* P-53: CDATA empty — document parses. */
static void test_p53_cdata_empty(void **state) {
	(void)state;
	SOURCE(s, "<r><![CDATA[]]></r>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	xml_document_free(doc, true);
}

/* P-54: CDATA with newline. */
static void test_p54_cdata_newline(void **state) {
	(void)state;
	SOURCE(s, "<r><![CDATA[line1\nline2]]></r>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	struct xml_string *content = xml_node_content(xml_document_root(doc));
	assert_non_null(content);
	/* Content must include a newline character. */
	size_t len = xml_string_length(content);
	uint8_t *buf = malloc(len + 1);
	assert_non_null(buf);
	xml_string_copy(content, buf, len);
	buf[len] = '\0';
	bool has_nl = (memchr(buf, '\n', len) != NULL);
	free(buf);
	assert_true(has_nl);
	xml_document_free(doc, true);
}

/* P-55: Adjacent CDATA sections — concatenated. */
static void test_p55_cdata_adjacent(void **state) {
	(void)state;
	SOURCE(s, "<r><![CDATA[a]]><![CDATA[b]]></r>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	assert_true(xml_str_eq(xml_node_content(xml_document_root(doc)), "ab"));
	xml_document_free(doc, true);
}

/* P-56: CDATA mixed with text — parses. */
static void test_p56_cdata_mixed_text(void **state) {
	(void)state;
	SOURCE(s, "<r>pre<![CDATA[data]]>post</r>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	xml_document_free(doc, true);
}

/* P-57: Unclosed CDATA — parse fails. */
static void test_p57_cdata_unclosed(void **state) {
	(void)state;
	SOURCE(s, "<r><![CDATA[unclosed</r>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_null(doc);
	free(s);
}

/* P-58: Namespace default xmlns attribute. */
static void test_p58_xmlns_default(void **state) {
	(void)state;
	SOURCE(s, "<r xmlns=\"http://example.com\"/>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	struct xml_node *root = xml_document_root(doc);
	assert_true(xml_node_attributes(root) >= 1);
	assert_true(xml_str_eq(xml_node_attribute_name(root, 0), "xmlns"));
	xml_document_free(doc, true);
}

/* P-59: Namespace prefixed xmlns attribute. */
static void test_p59_xmlns_prefixed(void **state) {
	(void)state;
	SOURCE(s, "<r xmlns:ns=\"http://example.com\"/>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	xml_document_free(doc, true);
}

/* P-60: Multiple namespace declarations — two attributes. */
static void test_p60_xmlns_multiple(void **state) {
	(void)state;
	SOURCE(s, "<r xmlns=\"u\" xmlns:n=\"v\"/>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	assert_int_equal(xml_node_attributes(xml_document_root(doc)), 2);
	xml_document_free(doc, true);
}

/* P-61: Tag name with underscore start. */
static void test_p61_underscore_start(void **state) {
	(void)state;
	SOURCE(s, "<_root/>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	xml_document_free(doc, true);
}

/* P-62: Tag name with colon start (accepted per XML Name production). */
static void test_p62_colon_start(void **state) {
	(void)state;
	SOURCE(s, "<:root/>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	xml_document_free(doc, true);
}

/* P-63: Tag name with letter then digits. */
static void test_p63_letter_then_digits(void **state) {
	(void)state;
	SOURCE(s, "<a123/>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	xml_document_free(doc, true);
}

/* P-64: Prefixed tag name. */
static void test_p64_prefixed_tag(void **state) {
	(void)state;
	SOURCE(s, "<ns:element/>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	xml_document_free(doc, true);
}

/* P-65: Attribute with single-quoted value. */
static void test_p65_single_quoted_attr(void **state) {
	(void)state;
	SOURCE(s, "<r a='v'/>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	assert_true(xml_str_eq(xml_node_attribute_content(xml_document_root(doc), 0), "v"));
	xml_document_free(doc, true);
}

/* P-66: Attribute with empty value. */
static void test_p66_empty_attr_value(void **state) {
	(void)state;
	SOURCE(s, "<r a=\"\"/>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	struct xml_string *val = xml_node_attribute_content(xml_document_root(doc), 0);
	assert_non_null(val);
	assert_int_equal(xml_string_length(val), 0);
	xml_document_free(doc, true);
}

/* P-67: Many attributes on one node. */
static void test_p67_many_attributes(void **state) {
	(void)state;
	SOURCE(s, "<r a=\"1\" b=\"2\" c=\"3\" d=\"4\" e=\"5\"/>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	assert_int_equal(xml_node_attributes(xml_document_root(doc)), 5);
	xml_document_free(doc, true);
}

/* P-68: Attribute with spaces in value. */
static void test_p68_attr_spaces_in_value(void **state) {
	(void)state;
	SOURCE(s, "<r a=\"hello world\"/>");
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_non_null(doc);
	assert_true(xml_str_eq(xml_node_attribute_content(xml_document_root(doc), 0), "hello world"));
	xml_document_free(doc, true);
}

/* P-69: realloc failure during parse — returns NULL; no crash.
 * Uses a document with an attribute so that realloc is triggered during parse. */
static void test_p69_realloc_failure(void **state) {
	(void)state;
	SOURCE(s, "<r a=\"v\"/>");
	will_return(__wrap_realloc, NULL);
	struct xml_document *doc = xml_parse_document(s, strlen((char const *)s));
	assert_null(doc);
	free(s);
}


static const struct CMUnitTest tests[] = {
	cmocka_unit_test(test_p01_minimal_self_closing),
	cmocka_unit_test(test_p02_minimal_open_close),
	cmocka_unit_test(test_p03_root_with_text),
	cmocka_unit_test(test_p04_root_with_child),
	cmocka_unit_test(test_p05_root_with_attribute),
	cmocka_unit_test(test_p06_xml_declaration),
	cmocka_unit_test(test_p07_xml_decl_utf8),
	cmocka_unit_test(test_p08_xml_decl_non_utf8),
	cmocka_unit_test(test_p09_comment_before_root),
	cmocka_unit_test(test_p10_multiple_comments),
	cmocka_unit_test(test_p11_pi_before_root),
	cmocka_unit_test(test_p12_multiple_pis),
	cmocka_unit_test(test_p13_pi_comment_mix),
	cmocka_unit_test(test_p14_doctype_minimal),
	cmocka_unit_test(test_p15_doctype_empty_subset),
	cmocka_unit_test(test_p16_doctype_system_id),
	cmocka_unit_test(test_p17_whitespace_only),
	cmocka_unit_test(test_p18_empty_buffer),
	cmocka_unit_test(test_p19_comment_no_root),
	cmocka_unit_test(test_p20_pi_no_root),
	cmocka_unit_test(test_p21_unclosed_tag),
	cmocka_unit_test(test_p22_mismatched_close),
	cmocka_unit_test(test_p23_stray_close),
	cmocka_unit_test(test_p24_unclosed_attr_quote),
	cmocka_unit_test(test_p25_digit_start_tag),
	cmocka_unit_test(test_p26_unescaped_amp_content),
	cmocka_unit_test(test_p27_unescaped_amp_end),
	cmocka_unit_test(test_p28_unescaped_amp_attr),
	cmocka_unit_test(test_p29_unknown_entity),
	cmocka_unit_test(test_p30_duplicate_attribute),
	cmocka_unit_test(test_p31_length_shorter),
	cmocka_unit_test(test_p32_length_exact),
	cmocka_unit_test(test_p33_length_larger),
	cmocka_unit_test(test_p34_deep_nesting),
	cmocka_unit_test(test_p35_wide_tree),
	cmocka_unit_test(test_p36_multiline_tag),
	cmocka_unit_test(test_p37_mixed_content),
	cmocka_unit_test(test_p38_entity_amp),
	cmocka_unit_test(test_p39_entity_lt),
	cmocka_unit_test(test_p40_entity_gt),
	cmocka_unit_test(test_p41_entity_quot),
	cmocka_unit_test(test_p42_entity_apos),
	cmocka_unit_test(test_p43_entity_mixed),
	cmocka_unit_test(test_p44_char_ref_decimal),
	cmocka_unit_test(test_p45_char_ref_hex),
	cmocka_unit_test(test_p46_char_ref_uppercase_x),
	cmocka_unit_test(test_p47_entity_in_attr),
	cmocka_unit_test(test_p48_char_ref_decimal_attr),
	cmocka_unit_test(test_p49_char_ref_hex_attr),
	cmocka_unit_test(test_p50_cdata_simple),
	cmocka_unit_test(test_p51_cdata_angle_brackets),
	cmocka_unit_test(test_p52_cdata_ampersand_literal),
	cmocka_unit_test(test_p53_cdata_empty),
	cmocka_unit_test(test_p54_cdata_newline),
	cmocka_unit_test(test_p55_cdata_adjacent),
	cmocka_unit_test(test_p56_cdata_mixed_text),
	cmocka_unit_test(test_p57_cdata_unclosed),
	cmocka_unit_test(test_p58_xmlns_default),
	cmocka_unit_test(test_p59_xmlns_prefixed),
	cmocka_unit_test(test_p60_xmlns_multiple),
	cmocka_unit_test(test_p61_underscore_start),
	cmocka_unit_test(test_p62_colon_start),
	cmocka_unit_test(test_p63_letter_then_digits),
	cmocka_unit_test(test_p64_prefixed_tag),
	cmocka_unit_test(test_p65_single_quoted_attr),
	cmocka_unit_test(test_p66_empty_attr_value),
	cmocka_unit_test(test_p67_many_attributes),
	cmocka_unit_test(test_p68_attr_spaces_in_value),
	cmocka_unit_test(test_p69_realloc_failure),
};

void get_ut_parse_document_tests(const struct CMUnitTest **out_tests, size_t *out_count) {
	*out_tests = tests;
	*out_count = sizeof(tests) / sizeof(tests[0]);
}
