# Testable issues: prioritized by likelihood and severity

This list includes only issues from the code review that are **reproducible or meaningfully testable with cmocka** (or with cmocka + ASan/Valgrind). Each issue has two scores:

- **Likelihood:** 1 = not likely, 2 = maybe, 3 = really likely  
- **Severity:** 1 = not an issue, 5 = total chaos  

Priority is by **severity (desc), then likelihood (desc)**. Suggested tests are described so you can add them to `test/unit-c.c`.

---

## Priority 1 — Severity 5, Likelihood 3 (really likely)

### 1. Public API NULL dereference (document, root, node, etc.)

| Item | Likelihood | Severity | Reproducible? |
|------|------------|----------|----------------|
| `xml_document_root(NULL)`, `xml_node_name(NULL)`, `xml_node_content(NULL)`, `xml_node_child(node, 0)` with NULL node, `xml_node_attributes(NULL)`, `xml_node_attribute_name` / `_content` with NULL, `xml_string_length(NULL)`, `xml_string_copy(NULL, ...)` | **3** | **5** | **Yes** |

**Why testable:** Call each function with NULL (and valid other args where needed); run test under Valgrind or expect no crash. After a fix (e.g. return 0/NULL or no-op), assert the defined behavior.

**Suggested test:**  
`test_api_null_document` — `xml_document_root(NULL)` → expect no crash; optionally assert returns NULL if you define that.  
`test_api_null_node` — `xml_node_name(NULL)`, `xml_node_content(NULL)`, `xml_node_children(NULL)`, etc.  
`test_api_null_string` — `xml_string_length(NULL)`, `xml_string_copy(NULL, buf, n)` (no-op or documented).

---

### 2. Tags with attributes: parsing fails (in-memory)

| Item | Likelihood | Severity | Reproducible? |
|------|------------|----------|----------------|
| `xml_parse_tag_end` stops at first space; opening tags like `<foo bar="baz">` never see `'>'`, so parse fails or attributes are never found | **3** | **5** | **Yes** |

**Why testable:** Parse in-memory XML with attributes via `xml_parse_document(source, len)` (e.g. `SOURCE(source, "<r><n a=\"v\">x</n></r>")`). Assert document is non-NULL, then assert the node `n` has one attribute with name `a` and content `v`. Current code either returns NULL document or node with 0 attributes.

**Suggested test:**  
`test_parse_attributes_in_memory` — SOURCE with `<Root><Node attr=\"value\">text</Node></Root>`, parse, assert 1 attribute on first child, name "attr", content "value". Fails today if the attribute bug is present.

---

## Priority 2 — Severity 5, Likelihood 2 (maybe)

### 3. Public API NULL (callers who pass NULL less often)

| Item | Likelihood | Severity | Reproducible? |
|------|------------|----------|----------------|
| `xml_easy_child(NULL, "x", 0)`, `xml_node_name_c_string(NULL)`, `xml_node_content_c_string(NULL)` (these already return 0 for NULL node in code, so may be safe) | **2** | **5** if wrong | **Yes** |

**Why testable:** Call with NULL node; assert no crash and (if you document it) return value 0/NULL.

**Suggested test:** Same as (1); include `xml_easy_child(NULL, "Tag", 0)`, `xml_node_name_c_string(NULL)`, `xml_node_content_c_string(NULL)` and assert they return NULL/0 and don’t crash.

---

## Priority 3 — Severity 4, Likelihood 3 (really likely)

### 4. `xml_document_free(NULL, ...)` — no NULL check

| Item | Likelihood | Severity | Reproducible? |
|------|------------|----------|----------------|
| `xml_document_free(document, bool)` dereferences `document` without checking for NULL | **3** | **4** | **Yes** |

**Why testable:** Call `xml_document_free(NULL, false)` and `xml_document_free(NULL, true)`; run under Valgrind or expect no crash. After fix (e.g. early return when `document == NULL`), test passes.

**Suggested test:**  
`test_document_free_null` — `xml_document_free(NULL, false);` and `xml_document_free(NULL, true);` with no assertion except “no crash” (or assert if you add a return value).

---

## Priority 4 — Severity 3–4, Likelihood 2 (maybe)

### 5. `xml_parser_error` buffer over-read when reporting error at end of buffer

| Item | Likelihood | Severity | Reproducible? |
|------|------------|----------|----------------|
| When `character == parser->length`, `parser->buffer[character]` is one past the end | **2** | **3** | **Yes (with ASan)** |

**Why testable:** Craft XML that triggers a parse error with the error position at the last byte (e.g. truncated or malformed so the parser reports error at end). Run the test under AddressSanitizer; without fix, ASan can report a one-past-end read.

**Suggested test:**  
`test_parse_error_at_end_of_buffer` — SOURCE with something that fails at the very end (e.g. `<root>` with no closing, or incomplete tag). Call `xml_parse_document(source, strlen(...))`, expect NULL. Run under ASan; must not read past end when printing error.

---

### 6. `xml_open_document`: `feof()` loop and possible extra/indeterminate read

| Item | Likelihood | Severity | Reproducible? |
|------|------------|----------|----------------|
| `while (!feof(source))` can do one extra iteration; no `ferror()` check | **2** | **3** | **Maybe** |

**Why testable:** Use a small file (or `fmemopen` with a fixed buffer) whose size and content are known; parse with `xml_open_document` and assert document content and length match expectations. With a crafted size (e.g. exactly one “chunk”), the feof bug might produce an extra byte or wrong length. Harder to trigger reliably without controlling the exact read path.

**Suggested test:**  
`test_open_document_exact_buffer` — Create a temp file with minimal valid XML (e.g. `<a/>`), open and parse with `xml_open_document`, assert root name is "a" and no extra garbage. Optionally compare `document->buffer.length` to file size.

---

## Priority 5 — Severity 4, Likelihood 1 (not likely in normal test)

### 7. `realloc` failure: leak and lost data

| Item | Likelihood | Severity | Reproducible? |
|------|------------|----------|----------------|
| `attributes = realloc(...)`, `children = realloc(...)`, `buffer = realloc(...)` — on NULL, original pointer is lost | **1** | **4** | **Yes (wrap realloc + will_return_ptr)** |

**Code status:** Fixed. All three sites use a temporary (`new_ptr = realloc(ptr, size)`); on NULL the original pointer is preserved, resources are freed or handed to existing cleanup, and no leak or double-free occurs.

**Testability:** Reproducible with cmocka by mocking `realloc` via the linker's `--wrap=realloc`: the test executable links with `wrap_realloc.c` (defining `__wrap_realloc`) and `LINKER:--wrap=realloc`. A test uses `will_return_ptr(__wrap_realloc, NULL)` so the next `realloc` returns NULL, then parses XML that triggers that realloc and asserts the document is NULL and (under Valgrind) no leak and no double-free.

---

## Summary table (testable only, by priority)

| Priority | Issue | Likelihood (1–3) | Severity (1–5) | Test type |
|----------|--------|-------------------|---------------|------------|
| 1 | API NULL (document/root/node/string) | 3 | 5 | Unit: call with NULL, no crash / defined return |
| 1 | Tags with attributes (in-memory parse) | 3 | 5 | Unit: parse XML with attrs, assert attribute count and content |
| 2 | API NULL (easy_* with NULL node) | 2 | 5 | Unit: same as above |
| 3 | `xml_document_free(NULL, ...)` | 3 | 4 | Unit: free(NULL), no crash |
| 4 | `xml_parser_error` buffer over-read | 2 | 3 | Unit + ASan: trigger error at end of buffer |
| 4 | `xml_open_document` feof / read error | 2 | 3 | Unit: small file, assert correct content/length |
| 5 | `realloc` on failure | 1 | 4 | Unit: wrap realloc, will_return_ptr NULL, assert no leak (Valgrind) |

---

## Likelihood scale (1–3)

- **1 – Not likely:** Requires rare conditions (e.g. OOM, custom allocator) or not triggerable in normal tests.
- **2 – Maybe:** Can happen in real use (e.g. NULL from caller, error at buffer end, specific file sizes).
- **3 – Really likely:** Common usage (NULL passed to API, any XML with attributes).

## Severity scale (1–5)

- **1 – Not an issue:** Cosmetic or documentation only.
- **2 – Minor:** Wrong only in edge cases or only affects diagnostics.
- **3 – Moderate:** Wrong behavior or UB in specific cases (e.g. error reporting, file read).
- **4 – Serious:** Crash/UB in cleanup or allocation path; possible data loss.
- **5 – Total chaos:** Core feature broken (e.g. attributes) or crash on common API use (NULL).
