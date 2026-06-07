#ifndef UT_HELPERS_H
#define UT_HELPERS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <xml.h>

/* Returns true iff the xml_string content equals the C string cstr. */
static bool __attribute__((unused)) xml_str_eq(struct xml_string *s, char const *cstr) {
	if (!s) { return cstr == NULL || cstr[0] == '\0'; }
	size_t slen = xml_string_length(s);
	size_t clen = strlen(cstr);
	if (slen != clen) { return false; }
	uint8_t *buf = malloc(slen + 1);
	if (!buf) { return false; }
	xml_string_copy(s, buf, slen);
	buf[slen] = '\0';
	bool eq = (memcmp(buf, cstr, slen) == 0);
	free(buf);
	return eq;
}

/* Allocates a writable copy of a string literal for xml_parse_document. */
#define SOURCE(name, literal)                                                  \
	uint8_t *name = calloc(strlen(literal) + 1, sizeof(uint8_t));              \
	{ char const *_lit = (literal); memcpy(name, _lit, strlen(_lit) + 1); }

#endif /* UT_HELPERS_H */
