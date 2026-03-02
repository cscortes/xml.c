# Changelog

This fork follows [semantic versioning](https://semver.org/). The version is based on the original [ooxi/xml.c](https://github.com/ooxi/xml.c) release **0.2.0** (latest upstream tag).

---

## [0.14.2] — 2026-03-01

### Changed

- **Programming style: NULL and explicit pointer checks** — [docs/programming_style.md](docs/programming_style.md) now requires using `NULL` (not `0`) for pointer null and explicit comparisons (`ptr == NULL`, `ptr != NULL`) in conditionals. Applied across [src/xml.c](src/xml.c) and [example/example.c](example/example.c), [example/example2.c](example/example2.c): all pointer returns use `return NULL`, pointer checks use `== NULL` / `!= NULL`, and zero-terminated array loops use `while (*p != NULL)`.

---

## [0.14.1] — 2026-03-01

### Added

- **Tests for xml_string_clone behaviour** — New [test/unit-c-string-clone.c](test/unit-c-string-clone.c) exercises the clone logic via the public `*_c_string` API: null-terminated copies, caller-owned buffers, attribute name/content clones, empty attribute value, and NULL node returning NULL. Wired into the C test runner and CMake.

### Fixed

- **xml_string_clone** — Return `NULL` (not `0`) for null input and when `calloc` fails; check `calloc` result to avoid undefined behaviour on allocation failure.

### Changed

- **Minor refactors in src/xml.c** — `get_zero_terminated_array_attributes` and `get_zero_terminated_array_nodes` rewritten from `while` to `for` loops.

---

## [0.14.0] — 2026-03-01

### Added

- **Second example (feature tour)** — [example/example2.c](example/example2.c) parses one XML document that demonstrates most features added in this fork: XML declaration (encoding UTF-8), DOCTYPE skip, processing instructions, comments, CDATA, entity/character references, namespaces, unique attributes, tag Name production, and APIs such as `xml_easy_child`, `xml_document_buffer_length`, and the c_string helpers. Each section of the output points to the relevant XML and the feature. Build target `xml-example2`; documented in README and [docs/quick_start.md](docs/quick_start.md).

---

## [0.13.1] — 2026-03-01

### Added

- **Test gap analysis and new test cases** — Added [docs/test_gap_analysis.md](docs/test_gap_analysis.md) summarizing gaps from the test spec and API. New tests: empty buffer and malformed parse (category 3); self-closing root, string length/copy, out-of-range child/attribute (categories 1 and 5); `xml_document_buffer_length(NULL)` (category 6). Second batch: empty element content NULL, partial/zero-length string copy, whitespace-only and malformed inputs, empty attribute value, duplicate-child easy_child, comment-only and PI-only documents, `xml_string_equals_cstr` NULL-as-empty, mixed content, many attributes, stray closing tag; encoding (version+encoding, unsupported case); CDATA entity literal; namespace three-prefixed; DOCTYPE PUBLIC/SYSTEM skipped; invalid entity in attribute. Test count 99 → 128.

---

## [0.13.0] — 2026-03-01

### Added

- **DOCTYPE / DTD handling (skip)** — Parser skips `<!DOCTYPE ...>` before the root element. Internal subset `[ ... ]` and quoted literals in the declaration are handled so the closing `>` is found correctly. No parsing of the DTD or resolution of external entities. See [docs/issues.md](docs/issues.md) and [test/unit-c-doctype.c](test/unit-c-doctype.c).
- **Encoding declaration (reject non-UTF-8)** — Parser reads the `encoding` attribute in `<?xml ...?>` and rejects the document if it is not UTF-8 (case-insensitive). Only UTF-8 is supported; no encoding conversion is performed. See [docs/issues.md](docs/issues.md) and [test/unit-c-encoding.c](test/unit-c-encoding.c).

### Changed

- **README encoding** — Encoding paragraph and compliance table updated to state that the XML declaration `encoding` is honored for rejection of non-UTF-8.

---

## [0.12.2] — 2026-03-01

### Changed

- **Per-feature test files** — Each feature now has its own test module. Split former `unit-c-compliance.c` into [test/unit-c-name-production.c](test/unit-c-name-production.c) (tag names), [test/unit-c-unique-attributes.c](test/unit-c-unique-attributes.c), [test/unit-c-ampersand-reject.c](test/unit-c-ampersand-reject.c) (standalone `&`), and [test/unit-c-namespace.c](test/unit-c-namespace.c). Added [test/unit-c-doctype.c](test/unit-c-doctype.c) and [test/unit-c-encoding.c](test/unit-c-encoding.c) for DOCTYPE/DTD and encoding declaration (docs/issues.md candidate features). Updated test runner, CMake, and docs (test_spec.md, quick_start_tests.md, issues.md) to reference the new modules.

---

## [0.12.1] — 2026-03-01

### Added

- **Compliance tests: standalone `&` and namespaces** — Tests for rejecting unescaped `&` in text and attribute values (docs/issues.md “Reject standalone `&` in content”) and for exposing `xmlns` / `xmlns:prefix` as attributes (namespace support candidate). See [test/unit-c-ampersand-reject.c](test/unit-c-ampersand-reject.c) and [test/unit-c-namespace.c](test/unit-c-namespace.c).

### Fixed

- **Parser cleanup on attribute entity error** — When entity/character reference expansion fails in an attribute value, the attribute’s content string is now initialized before free so that cleanup does not free uninitialized memory (avoids invalid free and Valgrind errors).

---

## [0.12.0] — 2026-03-01

### Added

- **Entity and character reference expansion** — Parser expands the five predefined entities (`&amp;` `&lt;` `&gt;` `&quot;` `&apos;`) and decimal/hex character references (`&#N;` / `&#xN;`) in element text and attribute values. Expansion runs during parse; `xml_node_content` and `xml_node_attribute_content` return UTF-8 with refs already expanded. Invalid refs cause parse failure. See [docs/issues.md](docs/issues.md) (XML compliance candidates).
- **Entity/character reference tests** — New [test/unit-c-entities.c](test/unit-c-entities.c) with 16 tests for predefined entities and character refs in content and attributes; wired into the C test runner and CMake.

### Changed

- **UTF-8 documented** — README, [src/xml.h](src/xml.h), [docs/xml_api.md](docs/xml_api.md), [docs/test_spec.md](docs/test_spec.md), [docs/quick_start.md](docs/quick_start.md), and [docs/issues.md](docs/issues.md) now state that the library assumes UTF-8 for all input and string data; no conversion or validation.
- **XML compliance table** — README reports **Yes** for entity references and character references (expanded in content and attributes).

---

## [0.11.0] — 2026-03-01

### Added

- **Stricter tag names (Name production)** — Parser rejects tag names that do not match an XML-like Name: must start with letter, `_`, or `:`; remaining characters may be letters, digits, `_`, `:`, `-`, `.`. Invalid names (e.g. `<2tag>`, `<.x>`) cause parse failure. Self-closing tags like `<r/>` are validated without the trailing `/`. See [docs/issues.md](docs/issues.md) (XML compliance candidates).
- **Unique attribute names per element** — Parser rejects duplicate attribute names on the same element (XML well-formedness requirement). Documents with repeated attribute names (e.g. `<e a="1" a="2">`) now fail to parse.
- **Compliance test modules** — New per-feature test files: [test/unit-c-name-production.c](test/unit-c-name-production.c) (tag-name validation: reject digit-start, accept letter/underscore/colon/prefixed) and [test/unit-c-unique-attributes.c](test/unit-c-unique-attributes.c) (reject duplicates, accept unique and single attributes). Wired into the C test runner and CMake.

### Changed

- **XML compliance table** — README "Current xml.c XML compliance" now reports **Yes** for "Tag names (Name production)" and "Unique attribute names per element".

---

## [0.10.1] — 2026-03-01

### Changed

- **Internal refactor** — Removed `xml_string_equals_internal`; public `xml_string_equals` now contains the full comparison logic (NULL checks + memcmp). Parser call sites use `xml_string_equals` directly.

---

## [0.10.0] — 2026-03-01

### Changed

- **API clarification / improvement** — Renamed the node text helpers from "easy" to "c_string" so the API clearly indicates they return 0-terminated C strings (caller must free): `xml_easy_name` → `xml_node_name_c_string`, `xml_easy_content` → `xml_node_content_c_string`. [src/xml.h](src/xml.h), [src/xml.c](src/xml.c), tests, and [docs/xml_api.md](docs/xml_api.md) updated. `xml_easy_child` is unchanged (it returns a node, not a string).

---

## [0.9.0] — 2026-03-01

### Added

- **CDATA sections (#40)** — Parser recognizes `<![CDATA[...]]>` and exposes content as character data (no markup or entity interpretation inside CDATA). Supports empty CDATA, CDATA in child elements, mixed text and CDATA, adjacent CDATA sections, and newlines; unclosed CDATA yields parse failure. New test module [test/unit-c-cdata.c](test/unit-c-cdata.c) with nine tests. [docs/issues.md](docs/issues.md), README, and [docs/test_spec.md](docs/test_spec.md) updated.

---

## [0.8.0] — 2026-03-01

### Added

- **Processing instructions (#30)** — Parser skips `<?...?>` PIs (including `<?xml ...?>`) before open/close tags and between nodes. [docs/issues.md](docs/issues.md) and README compliance table updated. New test module [test/unit-c-pi.c](test/unit-c-pi.c) with seven tests: XML decl before root, other PI before root, multiple PIs, PI between children, PI and comment mix, minimal PI, multiline PI.

### Changed

- **Test report cleanliness** — Expected parser error output is suppressed in two more tests: `test_open_document_empty_file` and `test_parse_exact_length_boundary` redirect stderr to `/dev/null` during the failing parse so messages like "length equals zero" and "expected <" do not appear in the test report (same pattern as existing error-path tests).

---

## [0.7.0] — 2026-03-01

### Added

- **XML comments (#21)** — Parser skips `<!-- ... -->` comments before open/close tags and between nodes. [docs/issues.md](docs/issues.md) and README updated; multiple unit tests added: comment before root, consecutive comments, empty/minimal comments, comment between text and closing tag, comment before self-closing child, root with only comment then close, multiline comments.

---

## [0.6.5] — 2026-03-01

### Fixed

- **#33 Attribute values with spaces** — Parser now uses quote-aware attribute parsing so values like `title="Hello World"` are parsed correctly (XML-compliant). Attribute boundaries are found by scanning for `name="value"` / `name='value'` instead of splitting on spaces. [docs/issues.md](docs/issues.md) and README "Fixes in this project" updated.

### Added

- **Test for attribute value with spaces** — `test_attribute_value_with_spaces` in [test/unit-c.c](test/unit-c.c) ensures one attribute with content `"Hello World"` is parsed from `<Node title="Hello World">`.

---

## [0.6.4] — 2026-03-01

### Added

- **Tiled/SVG-style test (#38)** — `test_tiled_svg_style_multiline_opening_tag`: parses XML with an opening tag that spans multiple lines and has attributes; would have caught upstream parse error. Parser now tokenizes attribute delimiters on space and newline (`XML_ATTRIBUTE_TOKEN_DELIMITERS()`).

### Changed

- **min/max → MIN/MAX** — Replaced `min`/`max` macros in [src/xml_common.h](src/xml_common.h) with `static inline` functions `MIN(size_t, size_t)` and `MAX(size_t, size_t)` (macro-style names). All call sites in [src/xml.c](src/xml.c) use `MIN`.
- **Attribute token delimiters** — Replaced literal `" \n\t\r"` in [src/xml.c](src/xml.c) with inline function `XML_ATTRIBUTE_TOKEN_DELIMITERS()`.
- **Style: 2 blank lines between functions** — [docs/programming_style.md](docs/programming_style.md) now requires two blank lines between function definitions; [src/xml.c](src/xml.c), [test/unit-c.c](test/unit-c.c), and [test/unit-c-null.c](test/unit-c-null.c) normalized to match. [docs/test_spec.md](docs/test_spec.md) lists the new Tiled/SVG test in the Attributes inventory.

---

## [0.6.3] — 2026-03-01

### Fixed

- **#31 Check missing headers** — Documented that the library header [src/xml.h](src/xml.h) is self-contained (includes stdbool.h, stdint.h, stdio.h, string.h). Example now explicitly includes `<string.h>`; README Usage describes required includes and adds `<string.h>` to the code sample. [docs/issues.md](docs/issues.md) and README "Fixes in this project" updated.

---

## [0.6.2] — 2026-03-01

### Fixed

- **CMake deprecation** — `example/CMakeLists.txt` and `test/CMakeLists.txt`: `cmake_minimum_required(VERSION 3.1.0 ...)` updated to `3.14` to match the root and silence the "Compatibility with CMake < 3.10 will be removed" warning.
- **xml_open_document** — Replaced `while (!feof(source))` with a fread-driven loop to avoid one extra iteration and indeterminate reads; added `ferror(source)` check after the read loop and `fclose(source)` return check so read/close errors are handled and the buffer is not leaked (see code_review.md §8–10).
- **xml_parser_consume** — When `position + n` exceeds buffer length, position is now set to `parser->length` (past end) instead of `parser->length - 1`; callers that read `parser->buffer[parser->position]` must check `position < length` (documented). Added guard in `xml_skip_whitespace` so we never read past end. Verbose debug path uses a safe copy length (code_review.md §12).

### Added

- **Unit tests for file and boundary logic** — `test_open_document_temp_file_exact_bytes`: temp file with exactly 8 bytes `"<a></a>"`, asserts document length and root name (fread/feof/fclose path). `test_open_document_empty_file`: empty file, expects NULL. `test_parse_exact_length_boundary`: parse `"<r></r>"` with length 8 (success) and `"<r>"` with length 3 (failure) to ensure no read past given length.

---

## [0.6.1] — 2026-03-01

### Fixed

- **Typos** — `src/xml.c`: "Read hole file" → "Read whole file". `example/example.c`: comment "than" → "then".
- **Compiler warnings** — With `-Wall -Wextra -Wpedantic`: type-limits in `xml_parser_error` (removed `max(0, min(...))` with `size_t`); unused parameter in `xml_find_attributes` (`UNUSED(parser)`); pointer-sign for `strlen(child_name)` and in example/unit-c (`uint8_t const*` casts). All targets (xml, example, test) now build warning-free.

### Added

- **Strict compiler options** — `-Wall -Wextra -Wpedantic` enabled for the xml library, example, and test targets in CMake.
- **xml_common.h** — New internal header with `min`, `max`, and `UNUSED` macros; used by `src/xml.c` and `example/example.c` (replaces local macro definitions).
- **docs/code_changes_before_after.md** — Before/after summary of the typo, warning, and macro changes.

---

## [0.6.0] — 2025-03-01

### Added

- **API documentation (Doxygen + Markdown)** — Build target `api_docs` generates HTML under `build/api_docs/html/` and writes [docs/xml_api.md](docs/xml_api.md) from Doxygen comments in `src/xml.h`. Requires Doxygen and Python 3; script [scripts/doxygen_xml_to_md.py](scripts/doxygen_xml_to_md.py) converts Doxygen XML to a single Markdown file. Commit workflow rule updated to list `docs/xml_api.md` as generated docs.

### Changed

- **Doxygen comments** — Header and source: corrected `xml_parse_buffer` → `xml_parse_document` in `xml_document_free` description; added `@param` for all public API in [src/xml.h](src/xml.h); fixed example blocks in [src/xml.c](src/xml.c) to use `\code`/`\endcode` so XML tags are not interpreted as HTML; fixed typos (e.g. "tag mismatch", "occurred", "allocated"). Test comments: [test/unit-c.c](test/unit-c.c) "eas" → "easy", [test/test_runner.h](test/test_runner.h) added `@param` for getter functions.

---

## [0.5.0] — 2025-03-01

### Added

- **realloc-failure test** — `test_realloc_failure_no_leak`: mocks `realloc` via linker `--wrap=realloc` and cmocka `will_return_ptr(__wrap_realloc, NULL)` so the next realloc fails; parses XML that triggers that realloc and asserts NULL document and no leak (Valgrind). Test helper [test/wrap_realloc.c](test/wrap_realloc.c) provides `__wrap_realloc`; test executable links with `LINKER:--wrap=realloc`. Documented in [docs/testable_issues_priority.md](docs/testable_issues_priority.md) §7 and [docs/test_spec.md](docs/test_spec.md) §3.
- **stderr suppression in error-path tests** — `test_parse_error_at_end_of_buffer` and `test_realloc_failure_no_leak` redirect stderr to `/dev/null` during the parse so expected `xml_parser_error` messages do not appear in the test output and confuse new developers.

### Fixed

- **realloc on failure (leak / lost data)** — All three realloc sites (attributes in `xml_find_attributes`, children in `xml_parse_node`, buffer in `xml_open_document`) now use a temporary and check for NULL; on failure the original pointer is preserved, resources are freed or handed to existing cleanup, and no leak or double-free occurs. Caller of `xml_find_attributes` checks for NULL and jumps to `exit_failure`.

---

## [0.4.0] — 2025-02-28

### Added

- **xml_document_buffer_length** — New public API: `size_t xml_document_buffer_length(struct xml_document* document)` returns the length in bytes of the buffer that was parsed (or 0 if document is NULL). Keeps `struct xml_document` opaque; useful for validation (e.g. comparing to file size when opened with `xml_open_document`). Declared in [src/xml.h](src/xml.h).
- **test_open_document_exact_buffer** — Parses `input/minimal.xml` (`<a></a>`) via `xml_open_document`; asserts root name `"a"`, no children, and `xml_document_buffer_length(document)` equals file size. Exercises the feof/read path (see docs/testable_issues_priority.md §6). Fixture [test/input/minimal.xml](test/input/minimal.xml); CMake copies it into the test build tree.

---

## [0.3.3] — 2025-02-28

### Fixed

- **xml_parser_error buffer over-read** — When reporting a parse error at the end of the buffer, `character` could equal `parser->length`; the code now avoids reading `parser->buffer[character]` in that case (use `'?'` instead) so AddressSanitizer does not report a one-past-end read.
- **Parser leak on parse failure** — When `xml_parse_node` failed after calling `xml_find_attributes`, the allocated `attributes` array (and its entries) was not freed in the `exit_failure` path; cleanup now frees attributes so Valgrind reports no leaks.

### Added

- **test_parse_error_at_end_of_buffer** — Malformed XML that triggers a parse error at end of buffer (e.g. `<root>` with no closing tag); expects NULL document. Run under ASan to verify no over-read in the error path. Documented in [docs/test_spec.md](docs/test_spec.md) §3.

---

## [0.3.2] — 2025-02-28

### Fixed

- **Opening tags with attributes** — Parser now uses `xml_parse_open_tag_content` for opening tags so that elements with attributes (e.g. `<Node attr="value">`) parse correctly instead of failing at the first space (see docs/testable_issues_priority.md §2).

### Added

- **Attribute tests (0, 1, 2)** — Six tests: `test_attributes_in_memory_0/1/2` (buffer) and `test_attributes_from_file_0/1/2` (fixtures). Fixtures `test/input/test-attributes-0.xml`, `test-attributes-1.xml`, `test-attributes-2.xml`; CMake copies them into the test build tree.

---

## [0.3.1] — 2025-02-28

### Added

- **NULL-safety in public API** — `xml_document_free`, `xml_document_root`, and all node/string accessors (`xml_node_name`, `xml_node_content`, `xml_node_children`, `xml_node_child`, `xml_node_attributes`, `xml_node_attribute_name`, `xml_node_attribute_content`, `xml_easy_child`) accept NULL and return safely (no-op or NULL/0) so callers do not crash.
- **NULL/lifecycle unit tests** — New test module `test/unit-c-null.c` (5 tests) plus single test runner (`test/test_main.c`, `test/test_runner.h`). All 11 tests run in one executable and produce one report.
- **Test docs** — [docs/quick_start_tests.md](docs/quick_start_tests.md): single combined report and 11-test layout; optional XML report artifact (cmocka `CMOCKA_MESSAGE_OUTPUT=xml`, `CMOCKA_XML_FILE`); tip to use `-V`/`--verbose` to see every test result regardless of failure.

### Changed

- **Test build** — Single C test executable `xml-test-c` built from `test_main.c` + `unit-c.c` + `unit-c-null.c`; `unit-c.c` no longer has its own `main()` (uses getter for runner). One CTest target, one Valgrind target.

### Fixed

- Segfaults when passing NULL to document or node APIs; all such calls now behave as documented (no crash, safe return value).

---

## [0.3.0] — modernization release

**Baseline:** [ooxi/xml.c release-0.2.0](https://github.com/ooxi/xml.c/releases/tag/release-0.2.0) (Dec 2017).

### Added

- **C-only test suite with cmocka** — Unit tests converted to [cmocka](https://cmocka.org/). CMake fetches and builds cmocka 2.0.0 via FetchContent; no system install or C++ compiler required.
- **Optional Valgrind test** — When Valgrind is installed, CTest runs **xml-test-c-valgrind** (memcheck, full leak check). Documented in [docs/quick_start_tests.md](docs/quick_start_tests.md) with install instructions.
- **Structured test layout** — `test/input/` for XML fixtures (`test.xml`, `test-attributes.xml`), `test/output/` for program output. Single C test executable `test/unit-c.c` (replaces former C and C++ test files).
- **Top-level example** — Example program moved to `example/` at project root with its own `example/CMakeLists.txt`.
- **Documentation** — [docs/quick_start.md](docs/quick_start.md) (build and run the example), [docs/quick_start_tests.md](docs/quick_start_tests.md) (run tests and Valgrind, install instructions). README and programming style updated for cmocka, Valgrind, and paths.

### Changed

- **Build** — CMake minimum 3.14 (for FetchContent). Project is C-only (`project(xml LANGUAGES C VERSION 0.3.0)` for CMake 3.28+ compatibility); C++ tests removed. Example built via `add_subdirectory(example)`.
- **Tests** — All tests consolidated into one C executable; assertions use cmocka (`assert_true`, `assert_non_null`, `assert_int_equal`, `assert_string_equal`, etc.). Test “find by tag name” (formerly in a separate file) merged into `unit-c.c`.

### Removed

- **C++ tests** — `test-xml-cpp.cpp` and C++ build support removed; coverage is in the C test suite.
- **Legacy test file** — `test-huitre39.c` removed; its cases moved into `unit-c.c` as `test_find_node_by_tag_name`.

### Fixed

- Parser and memory-safety fixes (e.g. EOF handling, bounds checks, realloc checks) as identified during modernization.
- User-facing message typo (“Could parse” → “Could not parse”) in example and tests.

---

## Version numbering

- **MAJOR** — Incompatible API or behaviour changes.
- **MINOR** — New features, tests, or docs; backward compatible.
- **PATCH** — Bug fixes only; backward compatible.

This release is **0.3.0** (MINOR bump from upstream 0.2.0) for the added test framework, docs, and build/layout improvements.
