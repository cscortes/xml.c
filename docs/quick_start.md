# Quick start: build and run the example

This guide gets you from zero to running the **xml-example** program. It assumes you're new to the project or to building C projects with CMake.

---

## What you need

Before building, you must have:

| Requirement | Why |
|-------------|-----|
| **A C compiler** (GCC or Clang) | The project is C only; CMake will use it to compile the library and example. |
| **CMake** (3.10 or later) | Used to generate the build files (e.g. Makefile) and drive the build. |

If either is missing, CMake will fail with errors like *"No CMAKE_C_COMPILER could be found"* or *"CMake not found"*.

### Installing the tools

- **Fedora / RHEL:**  
  `sudo dnf install gcc cmake`

- **Debian / Ubuntu:**  
  `sudo apt install build-essential cmake`

- **macOS:**  
  Install Xcode Command Line Tools (`xcode-select --install`), then install CMake (e.g. `brew install cmake`).

- **Windows:**  
  Install [Visual Studio Build Tools](https://visualstudio.microsoft.com/visual-cpp-build-tools/) or MinGW with GCC, and [CMake](https://cmake.org/download/). Use a "Developer" or "x64 Native Tools" prompt so the compiler is on `PATH`.

Check that they're available:

```bash
gcc --version
cmake --version
```

---

## Build the example (step by step)

We use an **out-of-tree** build: you configure and build inside a separate directory (e.g. `build/`), not in the source tree. That keeps the repo clean and avoids "could not load cache" or "Makefile: No such file or directory" when you run the build in the wrong place.

### 1. Go to the project root

```bash
cd /path/to/xml.c
```

(Replace with your actual path, e.g. `~/CODE/XML/xml.c`.)

### 2. Configure (create the build files)

This generates `CMakeCache.txt` and a `Makefile` (or equivalent) inside `build/`:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
```

- **First time:** You must run this. Without it, `cmake --build` will fail with "Makefile: No such file or directory" or "could not load cache."
- **Later:** Only run this again if you change CMake options or add/remove source files.

### 3. Build the example

```bash
cmake --build build --target xml-example
```

This compiles the xml library and the **xml-example** executable. The binary is created inside `build/` (often `build/xml-example` or `build/example/xml-example`, depending on your generator).

### 4. Run the example

```bash
./build/xml-example
```

If the executable was placed in a subdirectory (e.g. `build/example/`), run:

```bash
./build/example/xml-example
```

You should see output like: `Hello World` and a line about `Root/This` children.

---

## One-liner (after prerequisites)

From the project root, with a C compiler and CMake installed:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build --target xml-example && ./build/xml-example
```

Adjust the last part to `./build/example/xml-example` if your build puts the binary there.

---

## Build everything (library + example + tests)

To build the library, example, and tests, then run the tests:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```

Or with Make: `cd build && make && ctest --output-on-failure`.

---

## Troubleshooting

| Problem | Cause | Fix |
|--------|--------|-----|
| **"No CMAKE_C_COMPILER could be found"** | No C compiler installed or not on `PATH`. | Install GCC or Clang (see above) and ensure it’s on your `PATH`. |
| **"could not load cache"** | You ran `cmake --build .` from the **source** directory. | Run `cmake -B build ..` (or `cmake ..` from inside `build/`) first, then run `cmake --build build` (or `cmake --build .` from inside `build/`). |
| **"Makefile: No such file or directory"** | The build directory was never configured. | Run the **configure** step first: `cmake -B build -DCMAKE_BUILD_TYPE=Release` (from the project root). |
| **"No rule to make target 'xml-example'"** | Build files are from an old run and the target name changed, or you’re in the wrong directory. | Reconfigure: remove `build/` or run `cmake -B build ..` again, then `cmake --build build --target xml-example`. |

---

## Next steps

- **Use the library:** See [README](../README.md) for a short code example and API overview.
- **Run the test suite:** `cmake --build build && ctest --test-dir build --output-on-failure`
- **Development style:** See [programming_style.md](programming_style.md) for contribution guidelines.
