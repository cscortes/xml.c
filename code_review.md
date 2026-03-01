# Code review: programmatic issues

This document lists issues found by static analysis, compiler warnings, and code inspection. One fix was applied to unblock builds (CMake `LANGUAGES`); all other items are reported for maintainer action.

---

## Build / tooling

### 1. CMake: `project()` requires `LANGUAGES` before language names (fixed)

- **Where:** `CMakeLists.txt` line 3  
- **Issue:** With CMake 3.28+, `project(xml C VERSION 0.3.0)` triggers: “project with VERSION, DESCRIPTION or HOMEPAGE_URL must use LANGUAGES before language names.”  
- **Fix applied:** Use `project(xml LANGUAGES C VERSION 0.3.0)`.

---

## Compiler warnings (with `-Wall -Wextra -Wpedantic` on `src/xml.c`)

### 2. `xml_parser_error`: comparison of unsigned expression with `< 0` is always false

- **Where:** `src/xml.c` lines 323–324  
- **Issue:** `max(0, min(...))` with `size_t` (unsigned) makes `max(0, …)` always take the second argument; the comparison is pointless and triggers `-Wtype-limits`.  
- **Fix:** Remove the `max(0, …)` wrapper and use `min(parser->length, parser->position + (size_t)(offset < 0 ? 0 : offset))` or clamp `offset` before use so the type is consistent.

### 3. `xml_find_attributes`: unused parameter `parser`

- **Where:** `src/xml.c` line 444  
- **Issue:** `parser` is unused; triggers `-Wunused-parameter`.  
- **Fix:** Use `(void)parser;` or remove the parameter if the API can be changed; otherwise mark or suppress per project style.

### 4. `xml_easy_child`: pointer signedness with `strlen`

- **Where:** `src/xml.c` line 1032  
- **Issue:** `strlen(child_name)` with `child_name` as `uint8_t const*` triggers “pointer targets differ in signedness” (expects `const char*`).  
- **Fix:** Cast for the call, e.g. `strlen((char const*)child_name)`, or use a small helper that takes `uint8_t const*` and returns length without relying on `strlen`’s type.

---

## Null pointer and API contract

### 5. `xml_document_free`: no NULL check on `document`

- **Where:** `src/xml.c` lines 914–920  
- **Issue:** If `document` is NULL, `document->root` is undefined behavior.  
- **Fix:** Add an early return when `document == NULL` (and optionally document that NULL is allowed).

### 6. Other public API functions: no NULL checks

- **Where:** e.g. `xml_document_root`, `xml_node_name`, `xml_node_content`, `xml_node_child`, `xml_node_attributes`, `xml_node_attribute_name`, `xml_node_attribute_content`, `xml_string_length`, `xml_string_copy`, etc.  
- **Issue:** Dereferencing NULL pointer is undefined behavior.  
- **Fix:** Either document that pointers must be non-NULL and leave as-is, or add NULL checks and define behavior (e.g. return 0/NULL or no-op) for the public API.

---

## Memory and resource safety

### 7. `realloc` used without temporary: leak on failure

- **Where:**  
  - `src/xml.c` line 495: `attributes = realloc(attributes, ...)`  
  - `src/xml.c` line 758: `children = realloc(children, ...)`  
  - `src/xml.c` line 884: `buffer = realloc(buffer, ...)` in `xml_open_document`  
- **Issue:** If `realloc` fails, it returns NULL and the original pointer is lost, causing a leak and losing the existing data.  
- **Fix:** Use a temporary, e.g. `new_ptr = realloc(ptr, size); if (!new_ptr) { /* handle error */ } else { ptr = new_ptr; }`.

### 8. `xml_open_document`: `feof()` used as loop condition

- **Where:** `src/xml.c` line 879  
- **Issue:** Using `while (!feof(source))` is a common bug: `feof()` is true only after a read has failed, so the loop can run one extra time and use indeterminate data.  
- **Fix:** Loop on `fread` and break when bytes read is 0 (and optionally check `ferror(source)` for errors).

### 9. `xml_open_document`: no check for `fread` failure

- **Where:** `src/xml.c` lines 888–894  
- **Issue:** If `fread` fails (e.g. read error), `read` may be 0 or partial; the code still adds it to `document_length` and does not distinguish error from EOF.  
- **Fix:** Check `ferror(source)` after the loop and optionally treat short/zero read as error when appropriate.

### 10. `xml_open_document`: `fclose(source)` without checking return

- **Where:** `src/xml.c` line 896  
- **Issue:** `fclose` can fail (e.g. flush error); ignoring it may hide write/close errors.  
- **Fix:** Check return value and handle or log per project policy.

---

## Parsing logic and bounds

### 11. `xml_parser_error`: possible buffer over-read when reporting character

- **Where:** `src/xml.c` lines 313–314  
- **Issue:** `character` can equal `parser->length` (from `min(parser->length, parser->position + offset)`). Then `parser->buffer[character]` is one past the end.  
- **Fix:** Only pass `parser->buffer[character]` to `fprintf` when `character < parser->length`; otherwise use a placeholder (e.g. `'?'` or a separate message).

### 12. `xml_parser_consume`: clamping can hide over-consumption

- **Where:** `src/xml.c` lines 404–411  
- **Issue:** When `parser->position + n` exceeds `parser->length`, position is clamped to `parser->length - 1`. Callers may assume exactly `n` bytes were consumed; the parser state can be inconsistent and hide bugs.  
- **Fix:** Consider returning an error or not clamping (e.g. set to `parser->length`) and ensuring all callers handle “past end” consistently. At least document the clamping behavior.

### 13. `xml_parse_tag_end`: tags with attributes fail

- **Where:** `src/xml.c` lines 526–544  
- **Issue:** The loop stops at the first whitespace (e.g. after the tag name). For `<foo bar="baz">`, only “foo” is consumed; the parser is left at the space. The following check expects the current character to be `'>'`, so parsing fails.  
- **Impact:** Opening tags that have attributes (space before `>`) cannot be parsed; attribute parsing in `xml_find_attributes` never sees the attribute string because it only receives the tag name.  
- **Fix:** Either (a) parse the full opening tag (up to `'>'`) and pass that range to attribute parsing, or (b) advance the parser past attributes before expecting `'>'`, so that tags with attributes are supported.

### 14. `xml_find_attributes`: uses only tag name, not full opening tag

- **Where:** `src/xml.c` lines 462–468, 481–483  
- **Issue:** `tag_open` is the result of `xml_parse_tag_end`, so it is only the tag name (e.g. “foo”). `xml_string_clone(tag_open)` and `strtok` therefore only see the tag name; no attribute string is ever parsed. `position = token - tmp` and `start_name = &tag_open->buffer[position]` refer to the tag-name buffer, which does not contain `attr="value"`.  
- **Fix:** Depends on fixing (13): attribute finding must receive (or the parser must advance over) the full opening tag including attributes.

---

## Code quality and style

### 15. Typo in comment: “occured”, “allocalted”

- **Where:** `src/xml.c` line 797  
- **Issue:** “A failure occured, so free all allocalted resources” → “occurred”, “allocated”.

### 16. Typo in comment: “missmatch”

- **Where:** `src/xml.c` line 778 (error string)  
- **Issue:** “tag missmatch” → “tag mismatch”.

### 17. Typo in comment: “hole file”

- **Where:** `src/xml.c` line 877  
- **Issue:** “Read hole file” → “Read whole file”.

### 18. Typo in example comment: “than” → “then”

- **Where:** `example/example.c` line 58  
- **Issue:** “If you think this is a bug in xml.c, than use” → “then”.

### 19. `example.c`: unused parameters `argc`, `argv`

- **Where:** `example/example.c` line 30  
- **Issue:** Can trigger `-Wunused-parameter`.  
- **Fix:** Use `(void)argc; (void)argv;` or equivalent.

### 20. `example.c`: string literal assigned to `uint8_t*`

- **Where:** `example/example.c` lines 34–43  
- **Issue:** Literal is `char const*`; assigning to `uint8_t*` drops const and can trigger warnings.  
- **Fix:** Use `uint8_t const*` (or `char const*`) and adjust `xml_parse_document` if the API allows; or cast and document.

---

## Portability / headers

### 21. `xml.c`: `#include <malloc.h>` only when not `__MACH__`

- **Where:** `src/xml.c` lines 31–33  
- **Issue:** `malloc`/`free`/`realloc`/`calloc` are declared in `stdlib.h` in C; `malloc.h` is non-standard and not present on all platforms.  
- **Fix:** Rely on `stdlib.h` only (already included) and remove the `malloc.h` include if not needed for another reason.

### 22. `xml.h`: `size_t` without including `stddef.h`

- **Where:** `src/xml.h` (uses `size_t` in several declarations)  
- **Issue:** `size_t` is often obtained via `string.h` or other headers, but the standard way in C is `stddef.h`. Relying on indirect inclusion is fragile.  
- **Fix:** Add `#include <stddef.h>` to `xml.h` if `size_t` is not guaranteed by another included header.

---

## Summary

| Category           | Count |
|--------------------|-------|
| Build/tooling      | 1 (fixed) |
| Compiler warnings  | 3 |
| Null pointer / API | 2 |
| Memory/resource    | 4 |
| Parsing logic      | 4 |
| Style/typos        | 6 |
| Portability        | 2 |
| **Total**          | **22** |

Highest impact: (5) and (6) for robustness to NULL; (7) for leak on OOM; (13) and (14) for correct parsing of tags with attributes.
