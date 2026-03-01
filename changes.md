# Changelog

This fork follows [semantic versioning](https://semver.org/). The version is based on the original [ooxi/xml.c](https://github.com/ooxi/xml.c) release **0.2.0** (latest upstream tag).

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
