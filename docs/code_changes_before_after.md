# Code changes: before and after

Summary of edits from the typo fixes, warning fixes, and macro refactor.

---

## 1. Typos

### src/xml.c (~line 951)

**Before:**
```c
/* Read hole file */
```

**After:**
```c
/* Read whole file */
```

### example/example.c (comment ~line 57)

**Before:**
```c
* xml.c, than use a debug build
```

**After:**
```c
* xml.c, then use a debug build
```

---

## 2. Compiler options

### CMakeLists.txt (xml target)

**Before:**
```cmake
target_compile_options(
	xml
	PRIVATE
		-std=c11
)
```

**After:**
```cmake
target_compile_options(
	xml
	PRIVATE
		-std=c11
		-Wall
		-Wextra
		-Wpedantic
)
```

### example/CMakeLists.txt

**Before:**
```cmake
target_compile_options(
	"${PROJECT_NAME}-example"
	PRIVATE
		-std=c11
)
```

**After:**
```cmake
target_compile_options(
	"${PROJECT_NAME}-example"
	PRIVATE
		-std=c11
		-Wall
		-Wextra
		-Wpedantic
)
```

### test/CMakeLists.txt

**Before:**
```cmake
target_compile_options(
	"${PROJECT_NAME}-test-c"
	PRIVATE
		-std=c11
)
```

**After:**
```cmake
target_compile_options(
	"${PROJECT_NAME}-test-c"
	PRIVATE
		-std=c11
		-Wall
		-Wextra
		-Wpedantic
)
```

---

## 3. src/xml.c — warning fixes and macros

### Include and common header

**Before:**
```c
#include "xml.h"

#ifdef XML_PARSER_VERBOSE
```

**After:**
```c
#include "xml.h"
#include "xml_common.h"

#ifdef XML_PARSER_VERBOSE
```

### xml_parser_error: max(0, min(...)) and local min macro (lines 322–324)

**Before:**
```c
	#define min(X,Y) ((X) < (Y) ? (X) : (Y))
	#define max(X,Y) ((X) > (Y) ? (X) : (Y))
	size_t character = max(0, min(parser->length, parser->position + offset));
	#undef min
	#undef max
```

**After:**
```c
	size_t character = min(parser->length, parser->position + (size_t)(offset < 0 ? 0 : offset));
```

### xml_find_attributes: unused parameter (line 446)

**Before:**
```c
static struct xml_attribute** xml_find_attributes(struct xml_parser* parser, struct xml_string* tag_open) {
	xml_parser_info(parser, "find_attributes");
```

**After:**
```c
static struct xml_attribute** xml_find_attributes(struct xml_parser* parser, struct xml_string* tag_open) {
	UNUSED(parser);
	xml_parser_info(parser, "find_attributes");
```

### xml_parser_consume: local min macro (inside #ifdef XML_PARSER_VERBOSE)

**Before:**
```c
	#ifdef XML_PARSER_VERBOSE
	#define min(X,Y) ((X) < (Y) ? (X) : (Y))
	char* consumed = alloca((n + 1) * sizeof(char));
	memcpy(consumed, &parser->buffer[parser->position], min(n, parser->length - parser->position));
	consumed[n] = 0;
	#undef min
```

**After:**
```c
	#ifdef XML_PARSER_VERBOSE
	char* consumed = alloca((n + 1) * sizeof(char));
	memcpy(consumed, &parser->buffer[parser->position], min(n, parser->length - parser->position));
	consumed[n] = 0;
```

### xml_easy_child: strlen pointer-sign (child_name)

**Before:**
```c
		struct xml_string cn = {
			.buffer = child_name,
			.length = strlen(child_name)
		};
```

**After:**
```c
		struct xml_string cn = {
			.buffer = child_name,
			.length = strlen((char const*)child_name)
		};
```

### xml_string_copy: local min macro

**Before:**
```c
	if (!string) {
		return;
	}

	#define min(X,Y) ((X) < (Y) ? (X) : (Y))
	length = min(length, string->length);
	#undef min

	memcpy(buffer, string->buffer, length);
```

**After:**
```c
	if (!string) {
		return;
	}

	length = min(length, string->length);

	memcpy(buffer, string->buffer, length);
```

---

## 4. example/example.c

### Includes and main (UNUSED, source type, strlen)

**Before:**
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
		...
	;


	...
	struct xml_document* document = xml_parse_document(source, strlen(source));
```

**After:**
```c
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <xml.h>
#include <xml_common.h>

int main(int argc, char** argv) {
	UNUSED(argc);
	UNUSED(argv);

	/* XML source, could be read from disk
	 */
	uint8_t const* source = (uint8_t const*)""
		"<Root>"
		...
	;


	...
	struct xml_document* document = xml_parse_document((uint8_t*)source, strlen((char const*)source));
```

---

## 5. test/unit-c.c — xml_easy_child pointer-sign

**Before:**
```c
	struct xml_node* test_a = xml_easy_child(root, "This", "Is", "A", "Test", 0);
	...
	struct xml_node* test_b = xml_easy_child(root, "This", "Is", "B", "Test", 0);
	...
	struct xml_node* test_c = xml_easy_child(root, "This", "Is", "C", "Test", 0);
	...
	struct xml_node* must_be_null = xml_easy_child(root, "Child");
	...
	uint8_t* name_is = xml_easy_name(xml_easy_child(root, "This", "Is", 0));
	...
	struct xml_node* element = xml_easy_child(
		xml_document_root(document), "Element", "With", 0
	);
```

**After:**
```c
	struct xml_node* test_a = xml_easy_child(root, (uint8_t const*)"This", (uint8_t const*)"Is", (uint8_t const*)"A", (uint8_t const*)"Test", 0);
	...
	struct xml_node* test_b = xml_easy_child(root, (uint8_t const*)"This", (uint8_t const*)"Is", (uint8_t const*)"B", (uint8_t const*)"Test", 0);
	...
	struct xml_node* test_c = xml_easy_child(root, (uint8_t const*)"This", (uint8_t const*)"Is", (uint8_t const*)"C", (uint8_t const*)"Test", 0);
	...
	struct xml_node* must_be_null = xml_easy_child(root, (uint8_t const*)"Child");
	...
	uint8_t* name_is = xml_easy_name(xml_easy_child(root, (uint8_t const*)"This", (uint8_t const*)"Is", 0));
	...
	struct xml_node* element = xml_easy_child(
		xml_document_root(document), (uint8_t const*)"Element", (uint8_t const*)"With", 0
	);
```

---

## 6. New file: src/xml_common.h

**After (new file):**
```c
/**
 * Common macros for xml.c and its example/tests.
 * Not part of the public API.
 */
#ifndef XML_COMMON_H
#define XML_COMMON_H

#define min(X, Y) (((X) < (Y)) ? (X) : (Y))
#define max(X, Y) (((X) > (Y)) ? (X) : (Y))
#define UNUSED(x) ((void)(x))

#endif
```

---

## Summary

| File | Changes |
|------|--------|
| **src/xml.c** | Typo "hole"→"whole"; include `xml_common.h`; fix `max(0,min(...))` and use file-level `min`; `UNUSED(parser)`; `strlen((char const*)child_name)`; remove all local `#define min` / `#undef min`. |
| **src/xml_common.h** | New header with `min`, `max`, `UNUSED`. |
| **example/example.c** | Typo "than"→"then"; include `xml_common.h`; `UNUSED(argc)`/`UNUSED(argv)`; `uint8_t const* source` and casts for `xml_parse_document`/`strlen`. |
| **test/unit-c.c** | All `xml_easy_child` string arguments cast to `(uint8_t const*)"..."`. |
| **CMakeLists.txt** (root + example + test) | Add `-Wall -Wextra -Wpedantic` to the xml, example, and test targets. |
