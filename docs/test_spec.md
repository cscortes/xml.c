# Test spec: categories, inventory, and suggested tests

This document defines the test categories for the xml.c library, inventories the current tests, and lists suggested tests to fill gaps. Tests are C unit tests using [cmocka](https://cmocka.org/), built and run from the build directory so that `input/` fixtures are available.

---

## Scope and conventions

- **In scope:** Parsing well-formed XML (tags, nested elements, text content, attributes); public API behavior for valid inputs; behavior with NULL inputs and document lifecycle (free with/without buffer).
- **Out of scope:** CDATA, DTDs, and encoding conversions are not supported. Invalid-input tests only check that the parser returns NULL and does not crash (no guarantee on error messages).
- **Conventions:** The C suite is one executable (runner [test/test_main.c](../test/test_main.c) plus modules [test/unit-c.c](../test/unit-c.c) and [test/unit-c-null.c](../test/unit-c-null.c)); it produces one comprehensive cmocka report. Tests run with working directory = build directory; fixture files are in [test/input/](../test/input/).

---

## Categories

| Id | Category | Purpose |
|----|----------|---------|
| 1 | Parsing — valid (in-memory) | `xml_parse_document(buffer, len)` with well-formed XML; minimal doc, nested tags, text content. |
| 2 | Parsing — valid (from file) | `xml_open_document(FILE*)` with fixture files; correct root and structure. |
| 3 | Parsing — invalid / error | Empty buffer, malformed or truncated XML; expect NULL document; optional ASan for error-at-end-of-buffer. |
| 4 | Attributes | Nodes with attributes; count, name, content; in-memory and/or from file. |
| 5 | API — success | Each public function with valid inputs: document_root, node name/content, children/child, attributes, easy_child / easy_name / easy_content, string_length / string_copy. |
| 6 | API — NULL and lifecycle | NULL document/node/string; `xml_document_free(NULL, ...)`; no crash and (once defined) documented return values. |

---

## Per category: current inventory and suggested tests

### 1. Parsing — valid (in-memory)

**Current inventory**

| Test | File | Description | Fixture |
|------|------|-------------|---------|
| `test_xml_parse_document_0` | unit-c.c | Single tag; assert root name and content. | SOURCE (in-memory) |
| `test_xml_parse_document_1` | unit-c.c | Nested Parent/Child; children by index; content. | SOURCE (in-memory) |
| `test_xml_parse_document_2` | unit-c.c | Deep path; multiple children; easy_child, easy_name, easy_content. | SOURCE (in-memory) |
| `test_parse_attributes_in_memory` | unit-c.c | In-memory SOURCE; first child has 1 attribute "attr"/"value". | SOURCE (in-memory) |
| `test_find_node_by_tag_name` | unit-c.c | Depth-first search by tag name; multiple matches. | SOURCE (in-memory) |

**Suggested tests**

- Keep as-is.
- Optional: one minimal “single self-closing tag” test (e.g. `<a/>`) if that form is supported.

---

### 2. Parsing — valid (from file)

**Current inventory**

| Test | File | Description | Fixture |
|------|------|-------------|---------|
| `test_xml_parse_document_3` | unit-c.c | Opens file; root, Element, With, Child path. | input/test.xml |
| `test_attributes_from_file_0/1/2` | unit-c.c | Opens file; first child has 0, 1, or 2 attributes. | input/test-attributes-0.xml, -1.xml, -2.xml |
| `test_open_document_exact_buffer` | unit-c.c | Minimal file `<a></a>`; root "a", 0 children, buffer length equals file size. | input/minimal.xml |

**Suggested tests**

- Keep `test_xml_parse_document_3`.
- ~~Optional: `test_open_document_buffer_length`~~ — **implemented**: `test_open_document_exact_buffer` uses `xml_document_buffer_length()` to assert buffer length equals file size (locks feof/read behavior).

---

### 3. Parsing — invalid / error

**Current inventory**

| Test | File | Description | Fixture |
|------|------|-------------|---------|
| `test_parse_error_at_end_of_buffer` | unit-c.c | Malformed XML (e.g. `<root>` no closing); expect NULL; run under ASan to ensure no over-read in error path. | SOURCE (in-memory) |
| `test_realloc_failure_no_leak` | unit-c.c | Mock realloc to return NULL (cmocka `--wrap=realloc` + `will_return_ptr`); parse XML that triggers realloc; expect NULL document and no leak (Valgrind). | SOURCE (in-memory) |

**Suggested tests**

- `test_parse_empty_buffer` — length 0 (or NULL buffer if allowed); expect NULL document.
- `test_parse_malformed_returns_null` — e.g. `<root>` with no closing tag, or truncated tag; expect NULL.
- ~~Optional: `test_parse_error_at_end_of_buffer`~~ — **implemented**: malformed input so error is reported at the last byte; run under ASan to ensure no over-read in the error path.

---

### 4. Attributes

**Current inventory**

| Test | File | Description | Fixture |
|------|------|-------------|---------|
| `test_attributes_in_memory_0` | unit-c.c | In-memory; first child has 0 attributes. | SOURCE (in-memory) |
| `test_attributes_in_memory_1` | unit-c.c | In-memory; first child has 1 attribute. | SOURCE (in-memory) |
| `test_attributes_in_memory_2` | unit-c.c | In-memory; first child has 2 attributes. | SOURCE (in-memory) |
| `test_attributes_from_file_0` | unit-c.c | File; first child has 0 attributes. | input/test-attributes-0.xml |
| `test_attributes_from_file_1` | unit-c.c | File; first child has 1 attribute. | input/test-attributes-1.xml |
| `test_attributes_from_file_2` | unit-c.c | File; first child has 2 attributes. | input/test-attributes-2.xml |
| `test_tiled_svg_style_multiline_opening_tag` | unit-c.c | Tiled/SVG-style: opening tag spans multiple lines with attributes; would have caught upstream #38 (parse error). | SOURCE (in-memory) |

**Suggested tests**

- Both attribute tests are implemented; they verify attribute count, name, and content. No further tests required for this category.

---

### 5. API — success

**Current inventory**

| Test | File | Description | Fixture |
|------|------|-------------|---------|
| Spread across 0, 1, 2, 3, attributes, find_node | unit-c.c | Covered: document_root, node_name, node_content, node_children, node_child, node_attributes, attribute_name/content, easy_child, easy_name, easy_content. Not explicitly tested in isolation: `xml_string_length`, `xml_string_copy` (used only inside `string_equals`). | Various |

**Suggested tests**

- Optional: `test_string_length_and_copy` — parse minimal doc, get a node name, call `xml_string_length` and `xml_string_copy`, assert length and copied content. Rest is already covered.

---

### 6. API — NULL and lifecycle

Category 6 (NULL and lifecycle) tests live in a **separate test file**: [test/unit-c-null.c](../test/unit-c-null.c), so NULL-dereference coverage is isolated from the main parsing/API-success suite. This aligns with **Priority 1** in [testable_issues_priority.md](testable_issues_priority.md) (Public API NULL dereference, Likelihood 3, Severity 5).

**Current inventory**

| Test | File | Description | Fixture |
|------|------|-------------|---------|
| *(none)* in unit-c.c | — | No test in unit-c.c passes NULL to any API. Full coverage is in unit-c-null.c (see below). | — |

**Required tests for complete NULL-dereference coverage**

All of the following are implemented in [test/unit-c-null.c](../test/unit-c-null.c):

| Test | Scope |
|------|--------|
| `test_document_free_null` | `xml_document_free(NULL, false)` and `xml_document_free(NULL, true)`; no crash. |
| `test_api_null_document` | `xml_document_root(NULL)`; no crash; assert returns NULL once library is fixed. |
| `test_api_null_node` | For NULL node: `xml_node_name`, `xml_node_content`, `xml_node_children`, `xml_node_child(NULL, 0)`, `xml_node_attributes`, `xml_node_attribute_name(NULL, 0)`, `xml_node_attribute_content(NULL, 0)`; no crash; assert return 0/NULL. |
| `test_api_null_node_easy` | `xml_easy_child(NULL, "Tag", 0)`, `xml_easy_name(NULL)`, `xml_easy_content(NULL)`; no crash; assert return NULL. |
| `test_api_null_string` | `xml_string_length(NULL)`; no crash; assert 0. `xml_string_copy(NULL, buf, n)` with valid buffer; no crash; no-op. |

**API checklist (expected behavior when NULL is passed)**

| API | NULL arg | Expected behavior |
|-----|----------|-------------------|
| `xml_document_free(document, bool)` | document NULL | No crash; no-op (library must be fixed to check NULL). |
| `xml_document_root(document)` | document NULL | No crash; return NULL. |
| `xml_node_name(node)` | node NULL | No crash; return NULL. |
| `xml_node_content(node)` | node NULL | No crash; return NULL. |
| `xml_node_children(node)` | node NULL | No crash; return 0. |
| `xml_node_child(node, index)` | node NULL | No crash; return NULL. |
| `xml_node_attributes(node)` | node NULL | No crash; return 0. |
| `xml_node_attribute_name(node, index)` | node NULL | No crash; return NULL. |
| `xml_node_attribute_content(node, index)` | node NULL | No crash; return NULL. |
| `xml_easy_child(node, ...)` | node NULL | No crash; return NULL. |
| `xml_easy_name(node)` | node NULL | No crash; return NULL. |
| `xml_easy_content(node)` | node NULL | No crash; return NULL. |
| `xml_string_length(string)` | string NULL | No crash; return 0. |
| `xml_string_copy(string, buffer, length)` | string NULL | No crash; no-op. |

**Test helpers:** Helpers that take `xml_string*` (e.g. `string_equals` in unit-c.c) and are used with API return values that may be NULL should handle NULL (e.g. return false or a documented value) to avoid dereference inside the test.

---

## Fixtures

| File | Purpose |
|------|---------|
| [test/input/test.xml](../test/input/test.xml) | Minimal file-based document; no attributes; root, Prefix, Element, With, Child. |
| [test/input/test-attributes.xml](../test/input/test-attributes.xml) | Document with element that has attributes (`value="2"`, `value_2="Hello"`). |

---

## Reference

For severity and likelihood of the gaps above, and for suggested test implementation details, see [testable_issues_priority.md](testable_issues_priority.md).
