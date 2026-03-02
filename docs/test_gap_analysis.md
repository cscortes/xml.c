# Test gap analysis

This document summarizes the gap analysis between the test spec ([test_spec.md](test_spec.md)), the public API ([src/xml.h](../src/xml.h)), and the existing test suite. It lists missing tests that were identified and added to reach full coverage of the specified categories and API behavior.

---

## Current test count (before gap fill)

| File | Test count |
|------|------------|
| unit-c.c | 30 |
| unit-c-null.c | 7 |
| unit-c-pi.c | 7 |
| unit-c-cdata.c | 9 |
| unit-c-name-production.c | 7 |
| unit-c-unique-attributes.c | 4 |
| unit-c-ampersand-reject.c | 5 |
| unit-c-namespace.c | 3 |
| unit-c-entities.c | 17 |
| unit-c-doctype.c | 5 |
| unit-c-encoding.c | 5 |
| **Total** | **99** |

---

## Gaps identified

### Category 1 — Parsing valid (in-memory)

| Gap | Status | Notes |
|-----|--------|--------|
| Single self-closing root `<a/>` | **Added** | test_parse_self_closing_root in unit-c.c |

### Category 3 — Parsing invalid / error

| Gap | Status | Notes |
|-----|--------|--------|
| Empty buffer (length 0) → NULL | **Added** | test_parse_empty_buffer in unit-c.c |
| Malformed XML → NULL (explicit) | **Added** | test_parse_malformed_returns_null in unit-c.c (truncated tag, wrong close) |

### Category 5 — API success

| Gap | Status | Notes |
|-----|--------|--------|
| xml_string_length / xml_string_copy in isolation | **Added** | test_string_length_and_copy in unit-c.c |
| Out-of-range child index → NULL | **Added** | test_node_child_out_of_range in unit-c.c |
| Out-of-range attribute index → NULL | **Added** | test_node_attribute_out_of_range in unit-c.c (name/content and _c_string) |

### Category 6 — API NULL and lifecycle

| Gap | Status | Notes |
|-----|--------|--------|
| xml_document_buffer_length(NULL) → 0 | **Added** | test_api_null_document_buffer_length in unit-c-null.c |

---

## Tests added (summary)

1. **test_parse_empty_buffer** (unit-c.c) — `xml_parse_document(buf, 0)` returns NULL.
2. **test_parse_malformed_returns_null** (unit-c.c) — Malformed inputs (e.g. truncated `<`, wrong closing tag) return NULL.
3. **test_string_length_and_copy** (unit-c.c) — Parse minimal doc, get node name, call `xml_string_length` and `xml_string_copy`, assert length and content.
4. **test_parse_self_closing_root** (unit-c.c) — In-memory `<a/>`; root present, 0 children; root name as returned by parser (e.g. `a/`).
5. **test_node_child_out_of_range** (unit-c.c) — `xml_node_child(root, index)` with index ≥ child count returns NULL.
6. **test_node_attribute_out_of_range** (unit-c.c) — `xml_node_attribute_name`, `xml_node_attribute_content`, and their `_c_string` variants with out-of-range index return NULL.
7. **test_api_null_document_buffer_length** (unit-c-null.c) — `xml_document_buffer_length(NULL)` returns 0, no crash.

---

## Second batch (extended coverage)

Additional tests added to push coverage further:

### unit-c.c

| Test | Purpose |
|------|--------|
| test_node_content_empty_element | Empty/self-closing elements: xml_node_content and _c_string return NULL. |
| test_xml_string_copy_partial | xml_string_copy with buffer shorter than string (partial copy). |
| test_xml_string_copy_zero_length | xml_string_copy with length 0 (no write). |
| test_parse_whitespace_only_returns_null | Buffer with only whitespace, no root → NULL. |
| test_parse_malformed_unclosed_quote | Unclosed attribute quote → NULL. |
| test_attribute_empty_value | Attribute with empty value `""` parsed; length 0, equals_cstr "". |
| test_easy_child_returns_null_when_duplicate_children | Two children with same name → xml_easy_child returns NULL. |
| test_parse_only_comment_returns_null | Document only `<!-- ... -->` → NULL. |
| test_parse_only_pi_returns_null | Document only `<?xml ...?>` → NULL. |
| test_xml_string_equals_cstr_null_treated_as_empty | xml_string_equals_cstr(empty_string, NULL) true. |
| test_mixed_content_text_before_child | Mixed content: text before/after child concatenated. |
| test_many_attributes | Element with 5 attributes; count and last name/value. |
| test_parse_malformed_stray_close_returns_null | Stray `</orphan>` → NULL. |

### unit-c-encoding.c

| Test | Purpose |
|------|--------|
| test_encoding_version_and_encoding_utf8 | `<?xml version="1.0" encoding="UTF-8"?>` accepted. |
| test_encoding_unsupported_case_variants_rejected | `encoding="iso-8859-1"` rejected. |

### unit-c-cdata.c

| Test | Purpose |
|------|--------|
| test_cdata_entity_literal_not_expanded | CDATA content: `&amp;` etc. remain literal (no expansion). |

### unit-c-namespace.c

| Test | Purpose |
|------|--------|
| test_namespace_three_prefixed_exposed | Three xmlns:pre attributes; all exposed. |

### unit-c-doctype.c

| Test | Purpose |
|------|--------|
| test_doctype_public_system_skipped | DOCTYPE with PUBLIC and SYSTEM IDs skipped; root parsed. |

### unit-c-ampersand-reject.c

| Test | Purpose |
|------|--------|
| test_reject_invalid_entity_in_attribute | Invalid entity `&bad;` in attribute → NULL. |

---

## Current test count (after second batch)

| File | Test count |
|------|------------|
| unit-c.c | 43 |
| unit-c-null.c | 7 |
| unit-c-pi.c | 7 |
| unit-c-cdata.c | 10 |
| unit-c-name-production.c | 7 |
| unit-c-unique-attributes.c | 4 |
| unit-c-ampersand-reject.c | 6 |
| unit-c-namespace.c | 4 |
| unit-c-entities.c | 17 |
| unit-c-doctype.c | 6 |
| unit-c-encoding.c | 7 |
| **Total** | **128** |

---

## Reference

- [test_spec.md](test_spec.md) — Categories, inventory, suggested tests.
- [testable_issues_priority.md](testable_issues_priority.md) — Prioritized testable issues.
