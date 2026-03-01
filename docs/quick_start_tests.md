# Quick start: run the tests

This guide shows how to build and run the test suite. The project uses [cmocka](https://cmocka.org/) for C unit tests; CMake downloads and builds cmocka automatically—you don't need to install it. An optional [Valgrind](https://valgrind.org/) test runs the same C tests under memory checking (leak and error detection) when Valgrind is installed.

**Prerequisites:** Same as for building the project: a C compiler (GCC or Clang) and [CMake](https://cmake.org/) 3.14 or later. See [quick_start.md](quick_start.md) for install commands. For the optional Valgrind test, see [Installing Valgrind](#installing-valgrind) below.

---

## Run all tests (after one-time configure)

From the project root:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```

- **First time:** `cmake -B build ...` configures the project and fetches cmocka (may take a moment).
- **Later:** You can just run `cmake --build build && ctest --test-dir build --output-on-failure` after changing code.

You should see something like:

**Without Valgrind:**
```
Test project /path/to/xml.c/build
    Start 1: xml-test-c
1/1 Test #1: xml-test-c ...............   Passed    0.00 sec
100% tests passed, 0 tests failed out of 1
```

**With Valgrind installed:**
```
    Start 1: xml-test-c
1/2 Test #1: xml-test-c ...............   Passed    0.00 sec
    Start 2: xml-test-c-valgrind
2/2 Test #2: xml-test-c-valgrind .......   Passed    0.50 sec
100% tests passed, 0 tests failed out of 2
```

---

## What gets run

| Test | What it does |
|------|----------------|
| **xml-test-c** | Single C executable that runs all cmocka unit tests in one go (parsing, file-based, attributes, NULL/lifecycle). It is built from a runner (`test/test_main.c`) plus test modules (`test/unit-c.c`, `test/unit-c-null.c`) and produces **one comprehensive report** for the whole suite. |
| **xml-test-c-valgrind** | Same C tests run under [Valgrind](https://valgrind.org/) (memcheck, full leak check). Added only if Valgrind is installed; fails if Valgrind reports memory errors or leaks. |

The C test executable uses test input files from `test/input/` (copied into the build tree). Tests run with their working directory set to the build directory so they can find `input/test.xml` and `input/test-attributes.xml`.

---

## Run tests with verbose output

To see each cmocka test name and result:

```bash
ctest --test-dir build --output-on-failure -V
```

**Tip:** Use `-V` or `--verbose` if you want to see every test and its result (e.g. `[ RUN ]` / `[ OK ]`) regardless of whether any test fails; without it, CTest only shows full output when there is a failure.

Example output:

```
[==========] all: Running 17 test(s).
[ RUN      ] test_xml_parse_document_0
[       OK ] test_xml_parse_document_0
[ RUN      ] test_xml_parse_document_1
...
[  PASSED  ] 17 test(s).
```

---

## Produce a test report artifact

cmocka can write a **XML report** to a file so you get a persistent artifact (e.g. for CI or tooling). Run the test executable with two environment variables:

- **`CMOCKA_MESSAGE_OUTPUT=xml`** — use XML format (JUnit-style).
- **`CMOCKA_XML_FILE=<path>`** — write the report to this file (e.g. `build/test-results.xml`).

Example (from project root):

```bash
cd build/test
CMOCKA_MESSAGE_OUTPUT=xml CMOCKA_XML_FILE=test-results.xml ./xml-test-c
```

The file `build/test/test-results.xml` is created (or under whatever path you set). If the file cannot be created, cmocka falls back to stderr. With the single test runner, all tests appear in one XML report.

From the project root in one line:

```bash
(cd build/test && CMOCKA_MESSAGE_OUTPUT=xml CMOCKA_XML_FILE=test-results.xml ./xml-test-c); echo "Exit: $?"
```

CTest does not set these variables by default. To always produce an artifact when running CTest, you can set the test command to run the executable with the env vars, or run the binary directly as above after `ctest`.

---

## Run only the C test executable (no CTest)

You can run the test binary directly. It lives under the build directory (often `build/test/xml-test-c`):

```bash
./build/test/xml-test-c
```

Or from inside the build directory (tests expect to run from the build dir so they can find `input/`):

```bash
cd build
./test/xml-test-c
```

---

## Valgrind (memory-check test)

The test suite includes an optional **xml-test-c-valgrind** test. When [Valgrind](https://valgrind.org/) is installed, CMake adds this test automatically; it runs the same C unit tests under Valgrind with:

- `--tool=memcheck` — memory error detection
- `--leak-check=full` — full leak reporting
- `--track-origins=yes` — track uninitialised value origins
- `--error-exitcode=1` — CTest fails if Valgrind reports any error

Run all tests as usual; the Valgrind test runs with the others:

```bash
ctest --test-dir build --output-on-failure
```

If Valgrind is not installed, that test is not added—the other tests still run and CTest reports 100% passed for the tests that are present.

### Installing Valgrind

Valgrind is optional. Install it if you want the memory-check test.

- **Fedora / RHEL:**  
  `sudo dnf install valgrind`

- **Debian / Ubuntu:**  
  `sudo apt install valgrind`

- **macOS:**  
  `brew install valgrind`  
  (Valgrind on macOS has [limitations](https://valgrind.org/docs/manual/dist.readme-macos.html); the test may still be useful.)

- **Windows:**  
  Valgrind is not natively supported on Windows. Options:
  - Use [WSL](https://docs.microsoft.com/en-us/windows/wsl/) (e.g. Ubuntu) and install Valgrind there: `sudo apt install valgrind`, then build and run tests inside WSL.
  - Skip the Valgrind test; the main **xml-test-c** will still run.

Check that Valgrind is available:

```bash
valgrind --version
```

Reconfigure and run tests so CMake picks up Valgrind:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
ctest --test-dir build --output-on-failure
```

---

## Verify that tests are passing

After any change, reconfigure (if you changed CMake or added sources), rebuild, and run:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release   # only if needed
cmake --build build
ctest --test-dir build --output-on-failure
```

Check the last line: **100% tests passed, 0 tests failed** means everything is green.

---

## Troubleshooting

| Problem | What to do |
|--------|------------|
| **"No CMAKE_C_COMPILER could be found"** | Install a C compiler (e.g. `gcc`) and ensure it's on your `PATH`. See [quick_start.md](quick_start.md). |
| **"Could not load cache" / "Makefile: No such file or directory"** | Run the **configure** step first: `cmake -B build -DCMAKE_BUILD_TYPE=Release` from the project root. |
| **Tests fail with "Cannot open input/test.xml"** | Tests must run with working directory = build directory (so `input/` is there). CTest sets this automatically; if you run `./build/test/xml-test-c` by hand, run it from `build/` or `build/test/`. |
| **ctest reports "Not Run" for xml-test-c-valgrind** | Valgrind is optional. To add the test, [install Valgrind](#installing-valgrind) for your platform, then run `cmake -B build ...` again and `ctest --test-dir build`. If you don't need memory checking, you can ignore this—the main **xml-test-c** is what matters. |

---

## See also

- [quick_start.md](quick_start.md) — build and run the **example** program.
- [README](../README.md) — project overview and API usage.
- [programming_style.md](programming_style.md) — contribution guidelines.
