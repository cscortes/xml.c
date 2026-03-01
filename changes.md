# Changelog

This fork follows [semantic versioning](https://semver.org/). The version is based on the original [ooxi/xml.c](https://github.com/ooxi/xml.c) release **0.2.0** (latest upstream tag).

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
