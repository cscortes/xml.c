# Feature Test Cases — xml.c Public API

Exhaustive test cases covering every public API function. Organized by function, then integration scenarios.

---

## `xml_parse_document`

| # | Scenario | Input | Expected |
|---|---|---|---|
| P-01 | Minimal self-closing root | `<root/>` | Non-NULL document |
| P-02 | Minimal open/close root | `<root></root>` | Non-NULL document |
| P-03 | Root with text content | `<r>hello</r>` | Non-NULL; content = "hello" |
| P-04 | Root with child element | `<r><c/></r>` | Non-NULL; 1 child |
| P-05 | Root with attribute | `<r a="v"/>` | Non-NULL; 1 attribute |
| P-06 | XML declaration present | `<?xml version="1.0"?><r/>` | Non-NULL |
| P-07 | XML declaration + encoding UTF-8 | `<?xml version="1.0" encoding="UTF-8"?><r/>` | Non-NULL |
| P-08 | XML declaration + non-UTF-8 encoding | `<?xml version="1.0" encoding="ISO-8859-1"?><r/>` | NULL (rejected) |
| P-09 | Comment before root | `<!-- c --><r/>` | Non-NULL; root name = "r" |
| P-10 | Multiple comments before root | `<!-- a --><!-- b --><r/>` | Non-NULL |
| P-11 | PI before root | `<?pi data?><r/>` | Non-NULL |
| P-12 | Multiple PIs before root | `<?a?><?b?><r/>` | Non-NULL |
| P-13 | PI and comment mix before root | `<?pi?> <!-- c --> <r/>` | Non-NULL |
| P-14 | DOCTYPE minimal | `<!DOCTYPE r><r/>` | Non-NULL |
| P-15 | DOCTYPE with empty internal subset | `<!DOCTYPE r []><r/>` | Non-NULL |
| P-16 | DOCTYPE with PUBLIC/SYSTEM id | `<!DOCTYPE r SYSTEM "x.dtd"><r/>` | Non-NULL (skipped) |
| P-17 | Whitespace only | `   ` | NULL |
| P-18 | Empty buffer (`length=0`) | `""` | NULL |
| P-19 | Only a comment, no root | `<!-- no root -->` | NULL |
| P-20 | Only a PI, no root | `<?pi?>` | NULL |
| P-21 | Unclosed tag | `<root>` | NULL |
| P-22 | Mismatched close tag | `<root></wrong>` | NULL |
| P-23 | Stray close tag | `</root>` | NULL |
| P-24 | Unclosed attribute quote | `<root a="v>` | NULL |
| P-25 | Tag name starts with digit | `<1root/>` | NULL |
| P-26 | Unescaped `&` in content | `<r>foo & bar</r>` | NULL |
| P-27 | Unescaped `&` at end of content | `<r>foo &</r>` | NULL |
| P-28 | Unescaped `&` in attribute value | `<r a="a & b"/>` | NULL |
| P-29 | Unknown entity reference | `<r>&unknown;</r>` | NULL |
| P-30 | Duplicate attribute on same element | `<r a="1" a="2"/>` | NULL |
| P-31 | `length` shorter than actual content | buffer=`<root/>`, length=3 | NULL (truncated parse fails) |
| P-32 | `length` exactly matches content | buffer=`<r/>`, length=4 | Non-NULL |
| P-33 | `length` larger than actual content | buffer=`<r/>\0garbage`, length=20 | Non-NULL (parses up to valid end) |
| P-34 | Deep nesting (many levels) | `<a><b><c><d>v</d></c></b></a>` | Non-NULL; navigable |
| P-35 | Wide tree (many siblings) | `<r><a/><b/><c/><d/></r>` | Non-NULL; 4 children |
| P-36 | Multiline opening tag (Tiled/SVG style) | `<r\n  a="1"\n  b="2"\n/>` | Non-NULL; 2 attributes |
| P-37 | Mixed content (text + child elements) | `<r>text<c/>more</r>` | Non-NULL |
| P-38 | `&amp;` entity in content | `<r>&amp;</r>` | Non-NULL; content = "&" |
| P-39 | `&lt;` entity in content | `<r>&lt;</r>` | Non-NULL; content = "<" |
| P-40 | `&gt;` entity in content | `<r>&gt;</r>` | Non-NULL; content = ">" |
| P-41 | `&quot;` entity in content | `<r>&quot;</r>` | Non-NULL; content = `"` |
| P-42 | `&apos;` entity in content | `<r>&apos;</r>` | Non-NULL; content = "'" |
| P-43 | Mixed entities in content | `<r>&amp;&lt;&gt;</r>` | Non-NULL; content = "&<>" |
| P-44 | Decimal character reference in content | `<r>&#65;</r>` | Non-NULL; content = "A" |
| P-45 | Hex character reference in content | `<r>&#x41;</r>` | Non-NULL; content = "A" |
| P-46 | Uppercase hex char ref | `<r>&#X41;</r>` | NULL or Non-NULL (define expected behavior) |
| P-47 | Entity in attribute value | `<r a="&amp;"/>` | Non-NULL; attr content = "&" |
| P-48 | Char ref in attribute value (decimal) | `<r a="&#65;"/>` | Non-NULL; attr content = "A" |
| P-49 | Char ref in attribute value (hex) | `<r a="&#x41;"/>` | Non-NULL; attr content = "A" |
| P-50 | CDATA section simple | `<r><![CDATA[hello]]></r>` | Non-NULL; content = "hello" |
| P-51 | CDATA with angle brackets | `<r><![CDATA[<not-a-tag>]]></r>` | Non-NULL; content = "<not-a-tag>" |
| P-52 | CDATA with ampersand (not expanded) | `<r><![CDATA[&amp;]]></r>` | Non-NULL; content = "&amp;" (literal) |
| P-53 | CDATA empty | `<r><![CDATA[]]></r>` | Non-NULL; content = "" |
| P-54 | CDATA with newline | `<r><![CDATA[line1\nline2]]></r>` | Non-NULL; content includes newline |
| P-55 | CDATA adjacent sections | `<r><![CDATA[a]]><![CDATA[b]]></r>` | Non-NULL; content = "ab" |
| P-56 | CDATA mixed with text | `<r>pre<![CDATA[data]]>post</r>` | Non-NULL |
| P-57 | CDATA unclosed | `<r><![CDATA[unclosed</r>` | NULL |
| P-58 | Namespace default xmlns attribute | `<r xmlns="http://example.com"/>` | Non-NULL; xmlns exposed as attribute |
| P-59 | Namespace prefixed xmlns attribute | `<r xmlns:ns="http://example.com"/>` | Non-NULL; xmlns:ns exposed as attribute |
| P-60 | Multiple namespace declarations | `<r xmlns="u" xmlns:n="v"/>` | Non-NULL; 2 attributes |
| P-61 | Tag name with underscore start | `<_root/>` | Non-NULL |
| P-62 | Tag name with colon start | `<:root/>` | Non-NULL (accepted per XML Name production) |
| P-63 | Tag name with letter then digits | `<a123/>` | Non-NULL |
| P-64 | Prefixed tag name | `<ns:element/>` | Non-NULL |
| P-65 | Attribute with single-quoted value | `<r a='v'/>` | Non-NULL; attr content = "v" |
| P-66 | Attribute with empty value | `<r a=""/>` | Non-NULL; attr content = "" |
| P-67 | Many attributes on one node | `<r a="1" b="2" c="3" d="4" e="5"/>` | Non-NULL; 5 attributes |
| P-68 | Attribute with spaces in value | `<r a="hello world"/>` | Non-NULL; attr content = "hello world" |
| P-69 | `realloc` failure during parse | (mock realloc to fail) | NULL; no leak |

---

## `xml_open_document`

| # | Scenario | Expected |
|---|---|---|
| O-01 | Valid XML file | Non-NULL document |
| O-02 | File with XML declaration | Non-NULL document |
| O-03 | Empty file | NULL |
| O-04 | File with invalid XML | NULL |
| O-05 | File returns exact byte count | `xml_document_buffer_length` equals file size |
| O-06 | NULL FILE* passed | NULL (or defined crash behavior — document what actually happens) |

---

## `xml_document_free`

| # | Scenario | Expected |
|---|---|---|
| F-01 | NULL document, `free_buffer=false` | No crash |
| F-02 | NULL document, `free_buffer=true` | No crash |
| F-03 | Valid document from `xml_parse_document`, `free_buffer=false` | No crash; caller still owns original buffer |
| F-04 | Valid document from `xml_parse_document`, `free_buffer=true` | No crash; buffer freed (do not access after) |
| F-05 | Valid document from `xml_open_document`, `free_buffer=true` | No crash (internal buffer freed) |

---

## `xml_document_root`

| # | Scenario | Expected |
|---|---|---|
| R-01 | NULL document | NULL |
| R-02 | Valid document with `<root/>` | Non-NULL node; name = "root" |
| R-03 | Valid document with namespaced root `<ns:root/>` | Non-NULL; name = "ns:root" |
| R-04 | Root name compared with `xml_string_equals_cstr` | Returns true for matching cstr |

---

## `xml_document_buffer_length`

| # | Scenario | Expected |
|---|---|---|
| BL-01 | NULL document | 0 |
| BL-02 | Document parsed from 7-byte buffer | Returns 7 |
| BL-03 | Document opened from file | Returns file size in bytes |

---

## `xml_node_name`

| # | Scenario | Expected |
|---|---|---|
| NN-01 | NULL node | NULL |
| NN-02 | Root node `<root/>` | Non-NULL string; equals "root" |
| NN-03 | Child node `<child/>` | Non-NULL string; equals "child" |
| NN-04 | Node with prefix `<ns:tag/>` | Non-NULL string; equals "ns:tag" |
| NN-05 | Node with underscore name `<_tag/>` | Non-NULL string; equals "_tag" |
| NN-06 | Name length matches `xml_string_length` | Correct byte count |

---

## `xml_node_content`

| # | Scenario | Expected |
|---|---|---|
| NC-01 | NULL node | NULL |
| NC-02 | Self-closing element `<r/>` | NULL |
| NC-03 | Empty element `<r></r>` | NULL or empty (define expected) |
| NC-04 | Element with text `<r>hello</r>` | Non-NULL; equals "hello" |
| NC-05 | Element with entity `<r>&amp;</r>` | Non-NULL; equals "&" |
| NC-06 | Element with CDATA `<r><![CDATA[text]]></r>` | Non-NULL; equals "text" |
| NC-07 | Element with child nodes and no text `<r><c/></r>` | NULL |
| NC-08 | Mixed content `<r>text<c/>more</r>` | Non-NULL (first text portion or concatenated — define expected) |
| NC-09 | Content with multiple entities | Correct expanded string |
| NC-10 | Content is whitespace only `<r>   </r>` | Non-NULL; equals "   " |

---

## `xml_node_children`

| # | Scenario | Expected |
|---|---|---|
| CH-01 | NULL node | 0 |
| CH-02 | Self-closing node `<r/>` | 0 |
| CH-03 | Node with one child | 1 |
| CH-04 | Node with multiple children | Correct count |
| CH-05 | Node with content only (no children) `<r>text</r>` | 0 |
| CH-06 | Node with comment children (comments are skipped) | Comments do not appear in count |
| CH-07 | Deeply nested: count at each level | Each level returns correct count |

---

## `xml_node_child`

| # | Scenario | Expected |
|---|---|---|
| CK-01 | NULL node | NULL |
| CK-02 | Index 0 on node with 1 child | Returns the child |
| CK-03 | Index out of range (index == children count) | NULL |
| CK-04 | Index out of range (large index) | NULL |
| CK-05 | Second child of node with 3 children | Returns correct second child |
| CK-06 | Child node name is correct | `xml_node_name` on returned child matches expected |

---

## `xml_node_attributes`

| # | Scenario | Expected |
|---|---|---|
| AT-01 | NULL node | 0 |
| AT-02 | Node with no attributes | 0 |
| AT-03 | Node with 1 attribute | 1 |
| AT-04 | Node with 3 attributes | 3 |
| AT-05 | Node with namespace attributes (xmlns counts) | Namespace attributes counted |
| AT-06 | Node with 5 attributes | 5 |

---

## `xml_node_attribute_name`

| # | Scenario | Expected |
|---|---|---|
| AN-01 | NULL node | NULL |
| AN-02 | Valid index 0 | Non-NULL; correct name |
| AN-03 | Out-of-range index | NULL |
| AN-04 | Namespace attribute name `xmlns:ns` | Non-NULL; equals "xmlns:ns" |
| AN-05 | Attribute with colon in name | Correct name string |
| AN-06 | Name length via `xml_string_length` | Correct byte count |

---

## `xml_node_attribute_content`

| # | Scenario | Expected |
|---|---|---|
| AC-01 | NULL node | NULL |
| AC-02 | Valid index 0 | Non-NULL; correct value |
| AC-03 | Out-of-range index | NULL |
| AC-04 | Attribute with empty value `a=""` | Non-NULL; length = 0 |
| AC-05 | Attribute with spaces in value `a="hello world"` | Non-NULL; equals "hello world" |
| AC-06 | Attribute with entity `a="&amp;"` | Non-NULL; equals "&" |
| AC-07 | Attribute with char ref `a="&#65;"` | Non-NULL; equals "A" |
| AC-08 | Attribute with single-quote value `a='val'` | Non-NULL; equals "val" |

---

## `xml_easy_child`

| # | Scenario | Expected |
|---|---|---|
| EC-01 | NULL node | NULL |
| EC-02 | Single level, child exists | Returns child node |
| EC-03 | Single level, child does not exist | NULL |
| EC-04 | Two-level path, both exist | Returns grandchild node |
| EC-05 | Two-level path, intermediate missing | NULL |
| EC-06 | Two-level path, final missing | NULL |
| EC-07 | Three-level path | Returns correct node |
| EC-08 | Path to node with duplicate names at that level | NULL (ambiguous — multiple matching children) |
| EC-09 | NULL sentinel as only argument (zero-step path) | Returns the node itself |
| EC-10 | Child name is an empty string | NULL or defined behavior |
| EC-11 | Named child is unique but has siblings with different names | Returns correct unique child |

---

## `xml_node_name_c_string`

| # | Scenario | Expected |
|---|---|---|
| NS-01 | NULL node | NULL |
| NS-02 | Valid node | Non-NULL; null-terminated; correct content |
| NS-03 | Caller frees result | No double-free; valgrind clean |
| NS-04 | Two calls return independent copies | Modifying one does not affect the other |
| NS-05 | Node name with prefix `ns:tag` | C string = `"ns:tag\0"` |

---

## `xml_node_content_c_string`

| # | Scenario | Expected |
|---|---|---|
| CS-01 | NULL node | NULL |
| CS-02 | Node with no content (self-closing) | NULL |
| CS-03 | Node with content `<r>hello</r>` | Non-NULL; null-terminated; equals "hello" |
| CS-04 | Content with expanded entity `<r>&amp;</r>` | Equals "&" |
| CS-05 | Caller frees result | No leak |
| CS-06 | Two calls return independent copies | Independent |

---

## `xml_node_attribute_name_c_string`

| # | Scenario | Expected |
|---|---|---|
| ANS-01 | NULL node | NULL |
| ANS-02 | Out-of-range index | NULL |
| ANS-03 | Valid node + index | Non-NULL; null-terminated; correct name |
| ANS-04 | Namespace attribute `xmlns:n` | C string = `"xmlns:n\0"` |
| ANS-05 | Caller frees result | No leak |

---

## `xml_node_attribute_content_c_string`

| # | Scenario | Expected |
|---|---|---|
| ACS-01 | NULL node | NULL |
| ACS-02 | Out-of-range index | NULL |
| ACS-03 | Valid node + index | Non-NULL; null-terminated; correct value |
| ACS-04 | Attribute with empty value | Non-NULL; `strlen == 0` |
| ACS-05 | Attribute with entity value `&amp;` | Equals "&" |
| ACS-06 | Caller frees result | No leak |

---

## `xml_string_length`

| # | Scenario | Expected |
|---|---|---|
| SL-01 | NULL string | 0 (or defined behavior) |
| SL-02 | Node name "root" | 4 |
| SL-03 | Node content "hello world" | 11 |
| SL-04 | Empty content `<r></r>` | 0 |
| SL-05 | Expanded entity `&amp;` → "&" | 1 (byte length of expanded form) |
| SL-06 | Multi-byte UTF-8 string (e.g. `<r>é</r>`, 2 bytes) | 2 |

---

## `xml_string_copy`

| # | Scenario | Expected |
|---|---|---|
| SC-01 | Buffer exactly the string length | All bytes written; no null terminator added |
| SC-02 | Buffer larger than string | String bytes written; remainder of buffer untouched |
| SC-03 | Buffer smaller than string (`length < string length`) | At most `length` bytes written; no overflow |
| SC-04 | `length = 0` | Nothing written; no crash |
| SC-05 | NULL string | No crash; nothing written |
| SC-06 | Copy then compare with `memcmp` | Copied bytes match source bytes exactly |

---

## `xml_string_equals`

| # | Scenario | Expected |
|---|---|---|
| SE-01 | Both NULL | false |
| SE-02 | First NULL, second valid | false |
| SE-03 | First valid, second NULL | false |
| SE-04 | Two nodes with same name | true |
| SE-05 | Two nodes with different names | false |
| SE-06 | Same content, different lengths (impossible by construction — verify) | false |
| SE-07 | Same bytes, same length | true |
| SE-08 | Case-different strings "Foo" vs "foo" | false (case-sensitive) |

---

## `xml_string_equals_cstr`

| # | Scenario | Expected |
|---|---|---|
| SEC-01 | NULL string, non-NULL cstr | false |
| SEC-02 | Valid string, NULL cstr | false (NULL treated as empty string — true only if string is also empty) |
| SEC-03 | Valid string, matching cstr | true |
| SEC-04 | Valid string, non-matching cstr | false |
| SEC-05 | Valid string, cstr same bytes but with extra trailing char | false |
| SEC-06 | Case-different: "Root" vs "root" | false (case-sensitive) |
| SEC-07 | Empty xml_string vs empty cstr `""` | true |
| SEC-08 | Empty xml_string vs NULL cstr | true (NULL treated as empty) |
| SEC-09 | Prefixed name "ns:tag" vs "ns:tag" | true |

---

## Cross-API / Integration Scenarios

| # | Scenario | APIs Exercised |
|---|---|---|
| I-01 | Parse → root → name → equals_cstr | `xml_parse_document`, `xml_document_root`, `xml_node_name`, `xml_string_equals_cstr` |
| I-02 | Parse → root → child(0) → name | `xml_parse_document`, `xml_document_root`, `xml_node_children`, `xml_node_child`, `xml_node_name` |
| I-03 | Parse → easy_child path → content_c_string | `xml_parse_document`, `xml_document_root`, `xml_easy_child`, `xml_node_content_c_string` |
| I-04 | Open file → buffer_length matches file size | `xml_open_document`, `xml_document_buffer_length` |
| I-05 | Parse → attribute_name_c_string + attribute_content_c_string at same index | `xml_node_attribute_name_c_string`, `xml_node_attribute_content_c_string` |
| I-06 | Parse → string_length + string_copy + manual compare | `xml_string_length`, `xml_string_copy` |
| I-07 | Parse → string_equals on two nodes with same tag name | `xml_string_equals` with two `xml_node_name` results |
| I-08 | Parse with entity → content_c_string → verify expansion | Entity resolution end-to-end |
| I-09 | Parse CDATA → content → equals_cstr | CDATA end-to-end |
| I-10 | Parse namespace → attribute loop → name equals "xmlns:ns" | Namespace attribute enumeration |
| I-11 | `xml_document_free(doc, false)` — buffer outlives document | Memory safety: buffer accessible after free |
| I-12 | `xml_document_free(doc, true)` — buffer freed with document | Memory safety: do not access buffer after free |

---

**Total: ~200 test cases** across 21 API functions and 12 integration scenarios.
