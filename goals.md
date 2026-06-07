# Project Goals — xml.c

## Sprint Goals

From [README.md](README.md):

1. Add new features not in the original codebase
2. Add test cases for coverage and increase quality
3. Add documentation for end developers
4. Fix any bugs we find along the way
5. Increase minimal compatibility with XML standards
6. Use AI as a pair programmer

## Strategic Direction

From [README.md](README.md):

- **ANSI C (C89/C90) portability** — the codebase is currently C11; migrating to ANSI C is an explicit goal to support older compilers and embedded toolchains. **Status: open.**
- **C-only test suite** — already achieved with cmocka.

## Performance TODOs

From [src/xml.c](src/xml.c) lines 1558–1561:

File I/O is currently byte-at-a-time pending a fix to 4096-byte chunks:

```c
size_t const read_chunk = 1; // TODO 4096;
size_t buffer_size = 1;      // TODO 4069
```

**Status: open.**

## XML Compliance Roadmap

From [docs/issues.md](docs/issues.md):

All candidate XML 1.0 features are done. The only remaining item:

| Feature | Status |
|---------|--------|
| Entity references in content and attributes | Done (v0.12.0) |
| Character references (decimal/hex) | Done (v0.12.0) |
| Stricter tag names (Name production) | Done (v0.11.0) |
| Unique attribute names per element | Done (v0.11.0) |
| Reject standalone `&` in content | Done (v0.12.1) |
| Namespace support | Done |
| DOCTYPE / DTD handling | Done |
| Encoding declaration | Done |
| **XML 1.1** | **Deferred** — out of scope for current XML 1.0 focus |

## Test Coverage Goals

From [docs/testable_issues_priority.md](docs/testable_issues_priority.md) and [docs/test_gap_analysis.md](docs/test_gap_analysis.md):

Priority-based test goals by severity:

| Priority | Severity | Area |
|----------|----------|------|
| 1 | 5 | API NULL dereference, tags with attributes parsing |
| 2 | 5 | Edge-case NULL handling |
| 3 | 4 | `xml_document_free(NULL)` safety |
| 4 | 3–4 | Buffer over-read, file reading |
| 5 | 4 | realloc failure handling |

Current total: 128 tests across 11 test modules.

## Files Containing Goals

| File | Goal Type |
|------|-----------|
| [README.md](README.md) | Sprint goals, ANSI C direction, planned features |
| [src/xml.c](src/xml.c) | Performance TODOs (I/O buffer size) |
| [docs/issues.md](docs/issues.md) | XML compliance roadmap |
| [docs/testable_issues_priority.md](docs/testable_issues_priority.md) | Test coverage priorities |
| [docs/test_gap_analysis.md](docs/test_gap_analysis.md) | Test expansion tracking |

## Open Items

| Goal | Location | Status |
|------|----------|--------|
| ANSI C (C89/C90) migration | [README.md:70](README.md) | Open |
| I/O performance: read chunk 1 → 4096 | [src/xml.c:1558-1561](src/xml.c) | Open |
| XML 1.1 support | [docs/issues.md:44](docs/issues.md) | Deferred |
