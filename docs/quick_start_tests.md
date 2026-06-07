# Quick start: run the tests

This guide shows how to build and run the test suite. The project uses [cmocka](https://cmocka.org/) for C unit tests; CMake downloads and builds cmocka automatically—you don't need to install it separately.

**Prerequisites:** A C compiler (GCC or Clang) and [CMake](https://cmake.org/) 3.14 or later. See [quick_start.md](quick_start.md) for install commands.

---

## One-time setup

From the project root, configure and build:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

The first `cmake -B build` step fetches and builds cmocka (takes a moment the first time). After that, rebuilding is just `cmake --build build`.

---

## Run all tests

```bash
ctest --test-dir build --output-on-failure
```

Expected output:

```
Test project /path/to/xml.c/build
    Start 1: xml-example
1/7 Test #1: xml-example ......................   Passed    0.00 sec
    Start 2: xml-example2
2/7 Test #2: xml-example2 .....................   Passed    0.00 sec
    Start 3: xml-example-valgrind
3/7 Test #3: xml-example-valgrind .............   Passed    0.40 sec
    Start 4: xml-example2-valgrind
4/7 Test #4: xml-example2-valgrind ............   Passed    0.38 sec
    Start 5: xml-test-c
5/7 Test #5: xml-test-c .......................   Passed    0.00 sec
    Start 6: xml-test-features
6/7 Test #6: xml-test-features ................   Passed    0.00 sec
    Start 7: xml-test-c-valgrind
7/7 Test #7: xml-test-c-valgrind ..............   Passed    0.59 sec

100% tests passed, 0 tests failed out of 7

Label Time Summary:
lowlevel    =   0.00 sec*proc (1 test)
unit        =   0.00 sec*proc (1 test)

Total Test time (real) =   1.38 sec
```

The **Label Time Summary** at the bottom reflects the two test suite labels (see below). Valgrind tests appear only if Valgrind is installed.

---

## Two test suites and their labels

The project has two C test executables, each registered under a CTest label:

| Label | Executable | Tests | Description |
|-------|-----------|------:|-------------|
| `lowlevel` | `xml-test-c` | 128 | Low-level API tests and parser edge cases. Tests parsing, file I/O, attributes, NULL/lifecycle, realloc-failure, PIs, CDATA, name production, unique attributes, ampersand rejection, namespaces, entities, DOCTYPE, and encoding. |
| `unit` | `xml-test-features` | 208 | Feature/spec tests derived from `docs/FeatureTestCases.md`. Each test is named after a spec case (P-01, R-02, NC-10, …). |

### Run only the lowlevel suite

```bash
ctest --test-dir build -L lowlevel --output-on-failure
```

### Run only the feature/unit suite

```bash
ctest --test-dir build -L unit --output-on-failure
```

### Run both suites but skip examples and Valgrind

```bash
ctest --test-dir build -L "lowlevel|unit" --output-on-failure
```

---

## Verbose output (see each test name)

Add `-V` to see every test name and result as cmocka prints them:

```bash
ctest --test-dir build -V --output-on-failure
```

To scope verbose output to one suite:

```bash
ctest --test-dir build -L unit -V --output-on-failure
```

Example snippet:

```
[==========] feature-unit: Running 208 test(s).
[ RUN      ] test_p01_minimal_self_closing
[       OK ] test_p01_minimal_self_closing
[ RUN      ] test_p02_minimal_open_close
[       OK ] test_p02_minimal_open_close
...
[  PASSED  ] 208 test(s).
```

---

## Run the test binaries directly

You can bypass CTest and run an executable directly. Tests expect the working directory to be the build directory so they can find `input/`:

```bash
# From the project root
cd build && ./test/xml-test-c
cd build && ./test/xml-test-features
```

Or using absolute paths:

```bash
cd build
./test/xml-test-c
./test/xml-test-features
```

---

## Produce a JUnit XML report

cmocka can write a JUnit-style XML report. Set two environment variables before running:

```bash
cd build
CMOCKA_MESSAGE_OUTPUT=xml CMOCKA_XML_FILE=lowlevel-results.xml ./test/xml-test-c
CMOCKA_MESSAGE_OUTPUT=xml CMOCKA_XML_FILE=unit-results.xml     ./test/xml-test-features
```

Each command creates an XML file you can feed to CI tools (Jenkins, GitHub Actions test-reporter, etc.).

---

## Valgrind (memory-check test)

When [Valgrind](https://valgrind.org/) is installed, CMake automatically adds **xml-test-c-valgrind** (runs the lowlevel suite under memcheck with full leak detection). It runs alongside the other tests when you do `ctest --test-dir build`.

Valgrind flags used:

| Flag | Purpose |
|------|---------|
| `--tool=memcheck` | Memory error detection |
| `--leak-check=full` | Full leak reporting |
| `--track-origins=yes` | Track uninitialised-value origins |
| `--error-exitcode=1` | CTest fails if Valgrind reports any error |

### Installing Valgrind

- **Fedora / RHEL:** `sudo dnf install valgrind`
- **Debian / Ubuntu:** `sudo apt install valgrind`
- **macOS:** `brew install valgrind`
- **Windows:** Valgrind is not natively supported; use WSL (`sudo apt install valgrind` inside Ubuntu WSL) or skip the Valgrind test.

After installing, reconfigure so CMake picks it up:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
ctest --test-dir build --output-on-failure
```

Check Valgrind is found: `valgrind --version`.

---

## Rebuild and re-run after code changes

```bash
cmake --build build
ctest --test-dir build --output-on-failure
```

No need to reconfigure unless you add new source files or change `CMakeLists.txt`.

---

## Troubleshooting

| Problem | What to do |
|--------|------------|
| **"No CMAKE_C_COMPILER could be found"** | Install GCC/Clang and ensure it is on `PATH`. |
| **"Could not load cache"** | Run the configure step first: `cmake -B build -DCMAKE_BUILD_TYPE=Release`. |
| **Tests fail with "Cannot open input/test.xml"** | Run the test binary from the `build/` directory, not the project root. CTest does this automatically. |
| **Valgrind test not listed** | Install Valgrind, then reconfigure: `cmake -B build ...`. |
| **26 unit test failures** | Ensure you are on the latest commit; earlier versions had test assertions that didn't match actual library behavior for self-closing tag names. |

---

## See also

- [quick_start.md](quick_start.md) — build and run the example program.
- [FeatureTestCases.md](FeatureTestCases.md) — spec that drives the `unit`-labeled tests.
- [README](../README.md) — project overview and API usage.
- [programming_style.md](programming_style.md) — contribution guidelines.
