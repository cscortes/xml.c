Fixes and features tracked from [ooxi/xml.c](https://github.com/ooxi/xml.c) issues. Already-fixed items are listed in README under **Fixes in this project**; candidates and current features are below.

---

## Fixes (from upstream issues)

| Status | Upstream # | Title | Note |
|--------|------------|--------|------|
| **Already fixed** | #24 | unsigned expression &lt; 0 is always false | Fixed in this fork (0.6.1). |
| **Already fixed** | #26 | Parsing error with xml header and empty element | Closed upstream; this fork handles empty/self-closing elements and related parse paths. |
| **Already fixed** | #39 (part) | Does not handle new lines in tags | This fork parses full opening tags up to `'>'`, so multiline tags (e.g. SVG) work. |
| **Already fixed** | #18 | Possible memory leak | Closed upstream; this fork has realloc handling and safe `xml_document_free`. |
| **Already fixed** | #33 | Does not handle attributes with spaces in the content | This fork uses quote-aware parsing so attribute values may contain spaces (XML-compliant). |
| **Already fixed** | #38 | parse error (Tiled/SVG-style XML) | Resolved by full open-tag parsing (multiline), quote-aware attributes (#33), and attribute delimiters including newline. |
| **Already fixed** | #31 | Check missing headers | Documented: [src/xml.h](src/xml.h) states it is self-contained (stdbool, stdint, stdio, string); example and README include string.h and describe required headers. |

## Features (from upstream issues)

| Status | Upstream # | Title | Note |
|--------|------------|--------|------|
| **Current (beyond original)** | — | Opening tags with attributes; full open-tag parsing | This fork parses elements with attributes and multiline opening tags (original did not). |
| **Current (beyond original)** | — | API and robustness | e.g. `xml_document_buffer_length`, NULL-safe API, `xml_open_document`/parser fixes, tests, docs. |
| **Current (beyond original)** | #21 | XML comments `<!-- ... -->` | Parser skips comments before open/close tags and between nodes. |
| **Current (beyond original)** | #30 | Processing instructions `<?...?>` | Parser skips `<?xml ...?>` and other PIs before tags and between nodes. |
| **Current (beyond original)** | #40 | CDATA sections `<![CDATA[...]]>` | Parser recognizes `<![CDATA[...]]>` and exposes content as character data (no markup/entity interpretation). |
| **Current (beyond original)** | #25 | Easier text printing / helpers | Zero-terminated C-string copy helpers for node and attribute text (`xml_node_name_c_string`, `xml_node_content_c_string`, `xml_node_attribute_name_c_string`, `xml_node_attribute_content_c_string`); compare helpers `xml_string_equals` and `xml_string_equals_cstr`. |

---

## XML compliance: candidate features

Features we could add to improve alignment with XML 1.0 (well-formedness and common practice). See README "XML compliance" for current status. **Implementation difficulty:** 1 = easy, 5 = hardest to implement. **Useful:** 1 = not needed, 5 = necessary.

| Priority | Status | Feature | Implementation difficulty (1–5) | Useful (1–5) | Description |
|----------|--------|---------|--------------------------------|--------------|-------------|
| High | **Done** | **Entity references in content and attributes** | 2 | 5 | Expand the five predefined entities in text and attribute values: `&amp;` `&lt;` `&gt;` `&quot;` `&apos;`. Implemented in 0.12.0; expanded during parse in content and attributes. |
| High | **Done** | **Character references** | 2 | 5 | Expand decimal `&#N;` and hexadecimal `&#xN;` character references in element and attribute content. Implemented in 0.12.0; expanded to UTF-8 during parse. |
| Medium | Done | **Stricter tag names (Name production)** | 1 | 2 | Reject or constrain tag names to the XML Name production (e.g. start with letter, `_`, or `:`; then Name characters). Currently the parser accepts any characters until `>` or space (e.g. `<2tag>`). |
| Medium | Done | **Unique attribute names per element** | 1 | 3 | Reject duplicate attribute names on the same element (required by XML well-formedness). Implemented in 0.11.0. |
| Medium | **Done** | **Reject standalone `&` in content** | 2 | 4 | Unescaped `&` in text or attribute values causes parse failure (entity/character reference required). Implemented via existing entity expansion; tests in 0.12.1. |
| Lower | **Done** | **Namespace support** | 4 | 4 | `xmlns` and `xmlns:prefix` are parsed and exposed as normal attributes (see [test/unit-c-namespace.c](test/unit-c-namespace.c)); no separate namespace API or prefixed-name resolution. |
| Lower | Not started | **DOCTYPE / DTD handling** | 4 | 3 | Skip or optionally parse `<!DOCTYPE ...>`; if parsing, could support internal subset and predefined entities only (no external entities). |
| Lower | Not started | **Encoding declaration** | 2 | 3 | Honor `encoding` in `<?xml ...?>` for rejection of unsupported encodings or future conversion. **UTF-8 is the only assumed/supported encoding**; input and all string data are treated as UTF-8. |
| Lower | Not started | **XML 1.1** | 3 | 2 | Optional support for XML 1.1 name and character rules (e.g. NEL, control chars) if targeting 1.1. |
