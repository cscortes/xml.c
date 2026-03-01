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
