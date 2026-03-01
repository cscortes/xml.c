# xml.c

- Parses an XML subset (comparable to other minimal XML/markup parsers), in short an XML reader.
- Simple, small, and self-contained in one file.
- Easy to embed in other projects without large external dependencies.
- See **XML compliance** below for what is supported and what is not.

**This repository is a modernization of the original xml.c.** For new features, fixes, tests, and documentation, use this repo. The original project is credited below.

[![Build Status](https://github.com/ooxi/xml.c/actions/workflows/ci.yaml/badge.svg)](https://github.com/ooxi/xml.c/actions) *(upstream CI)*


## Credits

xml.c was originally written by [ooxi/xml.c](https://github.com/ooxi/xml.c). This fork continues development as a **modernization project**: we add features, tests, docs, and bug fixes while keeping the library small and embeddable. **To use or contribute to the modernized codebase, clone or link to this repository** ([cscortes/xml.c](https://github.com/cscortes/xml.c)).


## Goals (this sprint)

1. Add new features not in the original codebase.
2. Add test cases for coverage and increase quality.
3. Add documentation for end developers.
4. Fix any bugs we find along the way.
5. Increase the level of minimal compatibility with XML standards.
6. Use AI as a pair programmer to help with these changes.


## Prerequisites

Currently required to build:

- [CMake](https://cmake.org/) 3.14 or later (needed to fetch and build [cmocka](https://cmocka.org/) for tests)
- A C11-capable C compiler (e.g. GCC, Clang)
- No C++ compiler is required; the test suite is C-only.

**Broader audience (direction):** We want to reach more developers. Planned steps include:
- **Plain Makefile** — so you can build with just `make` and a C compiler, without CMake.
- **ANSI C (C89/C90)** — so the library builds on older compilers and embedded toolchains. The codebase is currently C11; moving to ANSI C is a goal for this modernization project.
- **C-only test suite** — the project uses [cmocka](https://cmocka.org/) for unit tests (see `test/unit-c.c`). CMake fetches and builds cmocka at a fixed version; no system install or C++ compiler is required.

## Downloads

To use this fork, clone this repository or add it as a [git submodule](https://git-scm.com/book/en/Git-Tools-Submodules). Development is on `master`.


## Building xml.c

Clone the repo and build with CMake:

    $ git clone https://github.com/cscortes/xml.c.git xml.c
    $ mkdir xml.c/build && cd xml.c/build
    $ cmake -DCMAKE_BUILD_TYPE=Release ..
    $ make && ctest --output-on-failure

For a debug build: `cmake -DCMAKE_BUILD_TYPE=Debug ..` then rebuild.


## Testing

The test suite is C-only (see [cmocka](https://cmocka.org/)). From the build directory:

    $ ctest --output-on-failure

If [Valgrind](https://valgrind.org/) is installed, CMake adds an extra test **xml-test-c-valgrind** that runs the C tests under Valgrind (memcheck, leak check). Valgrind is optional—if it's not installed, that test is simply not added. For full instructions and how to install Valgrind, see [docs/quick_start_tests.md](docs/quick_start_tests.md).


## Development (this fork)

This repo is developed against the fork at [cscortes/xml.c](https://github.com/cscortes/xml.c):

- **origin** — this fork: `https://github.com/cscortes/xml.c.git` (default for `git push` / `git pull`)
- **upstream** — original: `https://github.com/ooxi/xml.c.git`

To sync from the original: `git fetch upstream && git merge upstream/master`.

See [docs/programming_style.md](docs/programming_style.md) for style guidelines (including grammar for user-facing messages).


## Current xml.c XML compliance

xml.c parses an **XML-like subset** only. It is **not** a strict subset of [XML 1.0](https://www.w3.org/TR/xml/): it rejects some valid XML and accepts some input that is not well-formed XML. The following table summarizes compliance with the bare XML standard.

| Feature | XML 1.0 | xml.c |
|---------|---------|-------|
| **Document** | | |
| XML declaration `<?xml ...?>` | Optional | **No** — not recognized; may misparse or fail |
| Single root element | Required | **Yes** — one root element only |
| **Elements** | | |
| Tag names (Name production) | Letter / `_` / `:` start; then Name chars | **No** — any chars until `>` or space (e.g. accepts `<2tag>`) |
| Empty-element tags `<foo/>` | Allowed | **Yes** |
| Proper nesting / matching tags | Required | **Yes** |
| **Attributes** | | |
| `name="value"` or `name='value'` | Required | **Yes** |
| Unique attribute names per element | Required | **No** — duplicates accepted |
| Entity/character refs in values | Allowed, must be expanded | **No** — not expanded |
| **Content** | | |
| Text content | Allowed | **Yes** |
| Character references `&#N;` / `&#xN;` | Required to be expanded | **No** — not supported |
| Entity references `&amp;` `&lt;` etc. | Required in content when using `&` `<` etc. | **No** — raw `&` accepted (invalid per XML) |
| CDATA sections `<![CDATA[...]]>` | Allowed | **No** — not supported |
| **Other** | | |
| Comments `<!-- ... -->` | Allowed | **No** — not recognized; may misparse |
| Processing instructions `<?...?>` | Allowed | **No** — not recognized |
| DTD / DOCTYPE | Optional | **No** — not supported |
| Namespaces | Common practice (Namespaces in XML) | **No** — no namespace handling |
| Encoding declaration / conversion | Declared and applied | **No** — no conversion; input treated as raw bytes |

**Summary:** Use xml.c only for controlled, simple XML-like input (elements, attributes, text). For standards-compliant or arbitrary XML, use a full parser (e.g. libxml2, Expat).


## Usage

This example is also included in the repository ([example/example.c](example/example.c))
and will be built by default. Most of the code is C boilerplate, the important
functions are `xml_parse_document`, `xml_document_root`, `xml_node_name`,
`xml_node_content` and `xml_node_child` / `xml_node_children`.

```c
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <xml.h>



int main(int argc, char** argv) {

	/* XML source, could be read from disk
	 */
	uint8_t* source = ""
		"<Root>"
			"<Hello>World</Hello>"
			"<This>"
				"<Is>:-)</Is>"
				"<An>:-O</An>"
				"<Example>:-D</Example>"
			"</This>"
		"</Root>"
	;


	/* Parse the document
	 *
	 * Watch out: Remember not to free the source until you have freed the
	 *     document itself. If you have to free the source before, supply a
	 *     copy to xml_parse_document which can be freed together with the
	 *     document (`free_buffer' argument to `xml_document_free')
	 */
	struct xml_document* document = xml_parse_document(source, strlen(source));

	/* You _have_ to check the result of `xml_parse_document', if it's 0
	 * then the source could not be parsed. If you think this is a bug in
	 * xml.c, then use a debug build (cmake -DCMAKE_BUILD_TYPE=Debug) which
	 * will verbosely tell you about the parsing process
	 */
	if (!document) {
		printf("Error: Could not parse document.\n");
		exit(EXIT_FAILURE);
	}
	struct xml_node* root = xml_document_root(document);


	/* Say Hello World :-)
	 */
	struct xml_node* root_hello = xml_node_child(root, 0);
	struct xml_string* hello = xml_node_name(root_hello);
	struct xml_string* world = xml_node_content(root_hello);

	/* Watch out: `xml_string_copy' will not 0-terminate your buffers! (but
	 *     `calloc' will :-)
	 */
	uint8_t* hello_0 = calloc(xml_string_length(hello) + 1, sizeof(uint8_t));
	uint8_t* world_0 = calloc(xml_string_length(world) + 1, sizeof(uint8_t));
	xml_string_copy(hello, hello_0, xml_string_length(hello));
	xml_string_copy(world, world_0, xml_string_length(world));

	printf("%s %s\n", hello_0, world_0);
	free(hello_0);
	free(world_0);


	/* Extract amount of Root/This children
	 */
	struct xml_node* root_this = xml_node_child(root, 1);
	printf("Root/This has %lu children\n", (unsigned long)xml_node_children(root_this));


	/* Remember to free the document or you'll risk a memory leak
	 */
	xml_document_free(document, false);
}
```

The full API (e.g. `xml_open_document`, `xml_node_attributes`, `xml_easy_child`) is declared in [src/xml.h](src/xml.h).


## License

BSD-style (same terms as [libpng/zlib](https://github.com/ooxi/xml.c/blob/master/LICENSE)):

```
Copyright (c) 2012 ooxi/xml.c
    https://github.com/ooxi/xml.c

This software is provided 'as-is', without any express or implied warranty. In
no event will the authors be held liable for any damages arising from the use of
this software.

Permission is granted to anyone to use this software for any purpose, including
commercial applications, and to alter it and redistribute it freely, subject to
the following restrictions:

 1. The origin of this software must not be misrepresented; you must not claim
    that you wrote the original software. If you use this software in a product,
    an acknowledgment in the product documentation would be appreciated but is
    not required.

 2. Altered source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.

 3. This notice may not be removed or altered from any source distribution.
```

The full text is in [LICENSE](LICENSE) in this repository.
