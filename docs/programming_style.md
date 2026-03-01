# Programming style

Guidelines for consistent, clear code in this project. The rules below are derived from existing code in this repository; follow them when adding or changing code.

---

## Formatting and layout

- **Indentation:** Use **tabs** (not spaces) for indentation. All C and C++ sources use tabs.
- **Braces:** Opening brace `{` on the **same line** as the function or control statement (K&R style). Closing brace `}` on its own line.
- **Blank lines:** Use one or two blank lines between logical sections (e.g. between functions). Multiple blank lines are used sparingly to separate larger sections.
- **Line length:** Prefer readable line length; long conditions may be split with the operator at the start of the continuation line (e.g. `||\t('/' != ...)`).

## Naming

- **C identifiers:** Use **snake_case** for functions, variables, and macros (e.g. `xml_parse_document`, `tag_open`, `get_zero_terminated_array_nodes`).
- **Public API:** All public API symbols use the **`xml_`** prefix (e.g. `xml_parse_document`, `xml_node_name`, `xml_string_copy`). Internal/private helpers are `static` and often use the same prefix (e.g. `xml_parser_consume`, `xml_find_attributes`).
- **Tests:** Test functions are named **`test_`** + descriptive name, often with a numeric suffix (e.g. `test_xml_parse_document_0`, `test_xml_parse_document_1`).
- **Enums / constants:** Use **UPPER_SNAKE_CASE** for enum values and related constants (e.g. `NO_CHARACTER`, `CURRENT_CHARACTER`, `NEXT_CHARACTER`).

## C language usage

- **C standard:** Code is built with **C11** (`-std=c11`). Use C11 features where they help; avoid unnecessary C99/C11-isms where C89 is enough.
- **Null pointers:** Both **`0`** and **`NULL`** appear in the codebase for pointer null. Prefer **`0`** for pointer return values in the library for consistency with most of `xml.c`; use `NULL` where it improves clarity (e.g. in conditionals).
- **Boolean type:** Use **`_Bool`** in C (or `stdbool.h`’s `bool` where included); in C++ tests, use `bool`.
- **Error/cleanup flow:** Use **`goto`** for centralized error handling and cleanup (e.g. `goto exit_failure`, `goto cleanup`, `goto node_creation`) instead of deep nesting or repeated cleanup code.
- **Pointer style:** Pointers are written with the asterisk **attached to the type** in most of the codebase (e.g. `struct xml_node* node`, `uint8_t* buffer`). Local style sometimes uses `char *str`; either is acceptable if consistent in the same file.
- **Const:** Use **`const` after the type** when it improves clarity (e.g. `size_t const length`, `int const rs`). For pointer-to-const, `char const*` or `const char*` are both used; prefer consistency within a file.

## Comments and documentation

- **File header:** Source and header files start with the **standard BSD-style copyright block** (see any `src/*.c`, `example/example.c`, or `test/unit-c.c`). Do not remove or alter the notice.
- **Block comments:** Use **`/* ... */`** for block comments. Multi-line blocks often use a leading `/*` and a trailing ` */` on the next line (with a space before `*/`). Single-line comments can use `//` in C++ or where already used (e.g. TODOs).
- **API labels:** In the library, mark comment blocks with **[PRIVATE]**, **[PUBLIC API]**, or **[OPAQUE API]** so readers can tell visibility. **[OPAQUE API]** is used for types whose internals are not part of the public contract.
- **Doxygen-style:** Use **`@param`**, **`@return`**, **`@warning`**, **`@see`** in block comments for functions that are part of the public or internal API. Use **“iff”** for “if and only if” in descriptions (e.g. “@return the node iff parsing was successful”).
- **Examples in comments:** Use **`---( Example )---`** … **`---`** to show example input or output in comments (e.g. in `xml_parse_tag_open`).
- **Code/symbols in comments:** Use **backticks** for names and symbols (e.g. `buffer`, `tag_open`, `<tag_name>`).

## Tests and examples

- **Test framework:** C tests use [cmocka](https://cmocka.org/). Test functions have signature `void test_*(void **state)` and use cmocka assertions (`assert_true`, `assert_non_null`, `assert_null`, `assert_int_equal`, `assert_string_equal`, etc.). The suite is run via `cmocka_run_group_tests`.
- **Test data:** Use the **`SOURCE(source, "string")`** macro (or equivalent) in tests to build a null-terminated `uint8_t*` source for the parser. Frees are done explicitly where needed (e.g. `xml_document_free(document, true)`).
- **Test layout:** One test per function; no shared global state between tests.
- **Valgrind:** When [Valgrind](https://valgrind.org/) is installed, CMake adds an optional **xml-test-c-valgrind** test that runs the C tests under memcheck. See [quick_start_tests.md](quick_start_tests.md) for how to install and run it.

## Build and includes

- **Includes:** Group includes: system/standard first, then project headers. Use `#include <xml.h>` in code that uses the library (rely on build to set include path).
- **CMake:** Library is **C11**, built as a static library. Options (e.g. `XML_PARSER_VERBOSE`) are set via `target_compile_definitions`. Test directory is added via `add_subdirectory(test)`.

---

## User-facing messages

All text that users see (e.g. `printf`, `fprintf`, log output, error strings) must use **proper grammar** and consistent formatting.

### Grammar

- Use **complete, correct sentences** where possible.
- Use proper **negation**: e.g. “could **not** parse” (not “could parse” when you mean failure).
- End sentences with a **period** when the message is a full sentence.
- Use correct subject–verb agreement and standard spelling.

**Good:**

- `"Error: Could not parse document.\n"`
- `"Assertion failed: %s, in %s (%s:%i)\n"`
- `"All tests passed :-)\n"`

**Bad:**

- `"Could parse document"` when parsing *failed* (wrong negation; misleading).
- `"could not parse document"` without a period (incomplete as a sentence).
- Missing “Error:” (or similar) on stderr for real errors so users can tell errors from normal output.

### Consistency

- **Error messages** to the user: prefer a short prefix like `"Error: "` and end with a period.
- **Assertion / test messages**: can be shorter fragments; keep grammar correct (e.g. “Could not parse document”).
- Use the same wording for the same situation across the codebase (e.g. one standard phrase for parse failure).

### Where this applies

- `README.md` and other docs that show code with user-visible strings.
- Example and test programs (`example/example.c`, `test/unit-c.c`).
- Library code that prints to stdout/stderr (e.g. `xml_parser_error`, `xml_parser_info`).

When adding or changing any user-facing string, check it against this section.
