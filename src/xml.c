/**
 * Copyright (c) 2012 ooxi/xml.c
 *     https://github.com/ooxi/xml.c
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from the
 * use of this software.
 * 
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 *  1. The origin of this software must not be misrepresented; you must not
 *     claim that you wrote the original software. If you use this software in a
 *     product, an acknowledgment in the product documentation would be
 *     appreciated but is not required.
 * 
 *  2. Altered source versions must be plainly marked as such, and must not be
 *     misrepresented as being the original software.
 *
 *  3. This notice may not be removed or altered from any source distribution.
 */
#include "xml.h"
#include "xml_common.h"

#ifdef XML_PARSER_VERBOSE
#include <alloca.h>
#endif

#include <ctype.h>

#ifndef __MACH__
#include <malloc.h>
#endif

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>


/** Delimiters for splitting tag name and attr="value" tokens (space, newline, tab, CR). */
static inline const char* XML_ATTRIBUTE_TOKEN_DELIMITERS(void) {
	return " \n\t\r";
}


/*
 * public domain strtok_r() by Charlie Gordon
 *
 *   from comp.lang.c  9/14/2007
 *
 *      http://groups.google.com/group/comp.lang.c/msg/2ab1ecbb86646684
 *
 *     (Declaration that it's public domain):
 *      http://groups.google.com/group/comp.lang.c/msg/7c7b39328fefab9c
 */
static char* xml_strtok_r(char *str, const char *delim, char **nextp) {
	char *ret;

	if (str == NULL) {
		str = *nextp;
	}

	str += strspn(str, delim);

	if (*str == '\0') {
		return NULL;
	}

	ret = str;

	str += strcspn(str, delim);

	if (*str) {
		*str++ = '\0';
	}

	*nextp = str;

	return ret;
}


/**
 * [OPAQUE API]
 *
 * UTF-8 text. When buffer_owned is true, buffer was allocated and must be
 * freed with the string; otherwise it points into the document buffer.
 */
struct xml_string {
	uint8_t const* buffer;
	size_t length;
	bool buffer_owned;
};

/**
 * [OPAQUE API]
 *
 * An xml_attribute may contain text content.
 */
struct xml_attribute {
	struct xml_string* name;
	struct xml_string* content;
};

/**
 * [OPAQUE API]
 *
 * An xml_node will always contain a tag name, a 0-terminated list of attributes
 * and a 0-terminated list of children. Moreover it may contain text content.
 */
struct xml_node {
	struct xml_string* name;
	struct xml_string* content;
	struct xml_attribute** attributes;
	struct xml_node** children;
};

/**
 * [OPAQUE API]
 *
 * An xml_document simply contains the root node and the underlying buffer
 */
struct xml_document {
	struct {
		uint8_t* buffer;
		size_t length;
	} buffer;

	struct xml_node* root;
};


/**
 * [PRIVATE]
 *
 * Parser context
 */
struct xml_parser {
	uint8_t* buffer;
	size_t position;
	size_t length;
};

/**
 * [PRIVATE]
 *
 * Character offsets
 */
enum xml_parser_offset {
	NO_CHARACTER = -1,
	CURRENT_CHARACTER = 0,
	NEXT_CHARACTER = 1,
};


/**
 * [PRIVATE]
 *
 * @return Number of attributes in 0-terminated array
 */
static size_t get_zero_terminated_array_attributes(struct xml_attribute** attributes) {
	size_t elements = 0;

	while (attributes[elements]) {
		++elements;
	}

	return elements;
}


/**
 * [PRIVATE]
 *
 * @return Number of nodes in 0-terminated array
 */
static size_t get_zero_terminated_array_nodes(struct xml_node** nodes) {
	size_t elements = 0;

	while (nodes[elements]) {
		++elements;
	}

	return elements;
}


/**
 * [PRIVATE]
 */
static uint8_t* xml_string_clone(struct xml_string* s) {
	if (!s) {
		return 0;
	}

	uint8_t* clone = calloc(s->length + 1, sizeof(uint8_t));

	xml_string_copy(s, clone, s->length);
	clone[s->length] = 0;

	return clone;
}


/**
 * [PRIVATE]
 *
 * Frees the resources allocated by the string. When buffer_owned is true,
 * also frees the buffer (used for concatenated content e.g. CDATA).
 */
static void xml_string_free(struct xml_string* string) {
	if (!string)
		return;
	if (string->buffer_owned && string->buffer)
		free((void*)string->buffer);
	free(string);
}


/**
 * [PRIVATE]
 *
 * Frees the resources allocated by the attribute
 */
static void xml_attribute_free(struct xml_attribute* attribute) {
	if(attribute->name) {
		xml_string_free(attribute->name);
	}
	if(attribute->content) {
		xml_string_free(attribute->content);
	}
	free(attribute);
}


/**
 * [PRIVATE]
 * 
 * Frees the resources allocated by the node
 */
static void xml_node_free(struct xml_node* node) {
	xml_string_free(node->name);

	if (node->content) {
		xml_string_free(node->content);
	}

	struct xml_attribute** at = node->attributes;
	while(*at) {
		xml_attribute_free(*at);
		++at;
	}
	free(node->attributes);

	struct xml_node** it = node->children;
	while (*it) {
		xml_node_free(*it);
		++it;
	}
	free(node->children);

	free(node);
}


/**
 * [PRIVATE]
 *
 * Echos the parsers call stack for debugging purposes
 */
#ifdef XML_PARSER_VERBOSE
static void xml_parser_info(struct xml_parser* parser, char const* message) {
	fprintf(stdout, "xml_parser_info %s\n", message);
}
#else
#define xml_parser_info(parser, message) {}
#endif



/**
 * [PRIVATE]
 *
 * Echos an error regarding the parser's source to the console
 */
static void xml_parser_error(struct xml_parser* parser, enum xml_parser_offset offset, char const* message) {
	int row = 0;
	int column = 0;

	size_t character = MIN(parser->length, parser->position + (size_t)(offset < 0 ? 0 : offset));

	size_t position = 0; for (; position < character; ++position) {
		column++;

		if ('\n' == parser->buffer[position]) {
			row++;
			column = 0;
		}
	}

	if (NO_CHARACTER != offset) {
		/* character may be parser->length (end of buffer); avoid one-past-end read */
		uint8_t ch = (character < parser->length) ? parser->buffer[character] : (uint8_t)'?';
		fprintf(stderr,	"xml_parser_error at %i:%i (is %c): %s\n",
				row + 1, column, (char)ch, message
		);
	} else {
		fprintf(stderr,	"xml_parser_error at %i:%i: %s\n",
				row + 1, column, message
		);
	}
}


/**
 * [PRIVATE]
 *
 * Returns the n-th not-whitespace byte in parser and 0 if such a byte does not
 * exist
 */
static uint8_t xml_parser_peek(struct xml_parser* parser, size_t n) {
	size_t position = parser->position;

	while (position < parser->length) {
		if (!isspace(parser->buffer[position])) {
			if (n == 0) {
				return parser->buffer[position];
			} else {
				--n;
			}
		}

		position++;
	}

	return 0;
}


/**
 * [PRIVATE]
 *
 * Moves the parser's position forward by n bytes. If the new position would
 * be past the buffer end, it is set to parser->length (past end). Callers
 * must not assume exactly n bytes were consumed when position reaches or
 * exceeds length; code that reads parser->buffer[parser->position] must
 * check position < parser->length first.
 */
static void xml_parser_consume(struct xml_parser* parser, size_t n) {

	/* Debug information
	 */
	#ifdef XML_PARSER_VERBOSE
	size_t safe = (parser->position < parser->length)
		? MIN(n, parser->length - parser->position) : 0;
	char* consumed = alloca((n + 1) * sizeof(char));
	if (safe > 0)
		memcpy(consumed, &parser->buffer[parser->position], safe);
	consumed[safe] = 0;

	size_t message_buffer_length = 512;
	char* message_buffer = alloca(512 * sizeof(char));
	snprintf(message_buffer, message_buffer_length, "Consuming %li bytes \"%s\"", (long)n, consumed);
	message_buffer[message_buffer_length - 1] = 0;

	xml_parser_info(parser, message_buffer);
	#endif


	/* Move the position forward; clamp to past end (length) if out of bounds
	 */
	parser->position += n;
	if (parser->position >= parser->length) {
		parser->position = parser->length;
	}
}


/**
 * [PRIVATE]
 * 
 * Skips to the next non-whitespace character
 */
static void xml_skip_whitespace(struct xml_parser* parser) {
	xml_parser_info(parser, "whitespace");

	if (parser->position >= parser->length)
		return;
	while (isspace(parser->buffer[parser->position])) {
		if (parser->position + 1 >= parser->length) {
			return;
		} else {
			parser->position++;
		}
	}
}


/**
 * [PRIVATE]
 * Skips an XML comment \c <!-- ... --> if the parser is positioned at \c <!--.
 * (docs/issues.md #21)
 *
 * @return true if a comment was skipped, false if not at \c <!-- or on error
 */
static bool xml_skip_comment(struct xml_parser* parser) {
	xml_parser_info(parser, "comment");
	if (parser->position + 4 > parser->length)
		return false;
	if (parser->buffer[parser->position] != '<'
	    || parser->buffer[parser->position + 1] != '!'
	    || parser->buffer[parser->position + 2] != '-'
	    || parser->buffer[parser->position + 3] != '-')
		return false;
	parser->position += 4;
	while (parser->position + 3 <= parser->length) {
		if (parser->buffer[parser->position] == '-'
		    && parser->buffer[parser->position + 1] == '-'
		    && parser->buffer[parser->position + 2] == '>') {
			parser->position += 3;
			return true;
		}
		parser->position++;
	}
	xml_parser_error(parser, CURRENT_CHARACTER, "xml_skip_comment::comment not closed (missing -->)");
	return false;
}


/**
 * [PRIVATE]
 * Skips an XML processing instruction \c <? ... ?> if the parser is positioned at \c <?.
 * Includes the XML declaration \c <?xml ...?> (docs/issues.md #30).
 *
 * @return true if a PI was skipped, false if not at \c <? or on error
 */
static bool xml_skip_processing_instruction(struct xml_parser* parser) {
	xml_parser_info(parser, "processing_instruction");
	if (parser->position + 2 > parser->length)
		return false;
	if (parser->buffer[parser->position] != '<'
	    || parser->buffer[parser->position + 1] != '?')
		return false;
	parser->position += 2;
	while (parser->position + 2 <= parser->length) {
		if (parser->buffer[parser->position] == '?'
		    && parser->buffer[parser->position + 1] == '>') {
			parser->position += 2;
			return true;
		}
		parser->position++;
	}
	xml_parser_error(parser, CURRENT_CHARACTER, "xml_skip_processing_instruction::PI not closed (missing ?>)");
	return false;
}


/**
 * [PRIVATE]
 * Parses an XML CDATA section \c <![CDATA[...]]> if the parser is positioned at
 * \c <![CDATA[. (docs/issues.md #40)
 *
 * @return New xml_string with CDATA content (pointer into document buffer), or
 *     NULL if not at CDATA start or section is unclosed
 */
static struct xml_string* xml_parse_cdata_section(struct xml_parser* parser) {
	xml_parser_info(parser, "cdata");
	if (parser->position + 9 > parser->length)
		return NULL;
	if (parser->buffer[parser->position] != '<'
	    || parser->buffer[parser->position + 1] != '!'
	    || parser->buffer[parser->position + 2] != '['
	    || memcmp((char const*)&parser->buffer[parser->position + 3], "CDATA[", 6) != 0)
		return NULL;
	parser->position += 9;
	size_t start = parser->position;
	/* Find "]]>" */
	while (parser->position + 3 <= parser->length) {
		if (parser->buffer[parser->position] == ']'
		    && parser->buffer[parser->position + 1] == ']'
		    && parser->buffer[parser->position + 2] == '>') {
			size_t content_len = parser->position - start;
			parser->position += 3;
			struct xml_string* s = malloc(sizeof(struct xml_string));
			if (!s)
				return NULL;
			s->buffer = &parser->buffer[start];
			s->length = content_len;
			s->buffer_owned = false;
			return s;
		}
		parser->position++;
	}
	xml_parser_error(parser, CURRENT_CHARACTER, "xml_parse_cdata_section::CDATA not closed (missing ]]> )");
	return NULL;
}


/**
 * [PRIVATE]
 * Skip past any characters in delim (whitespace), return pointer to first non-delim or to end.
 */
static char* skip_delim(char* p, const char* delim) {
	return p + strspn(p, delim);
}

/**
 * [PRIVATE]
 * XML 1.0-ish Name validation (ASCII-only).
 *
 * This project does not currently implement full Unicode NameStartChar/NameChar
 * tables. For well-formedness improvements (docs/issues.md), we enforce a
 * pragmatic subset:
 * - NameStartChar: [A-Za-z] '_' ':'
 * - NameChar: NameStartChar | [0-9] | '-' | '.'
 *
 * Additionally, this parser historically includes a trailing '/' in the node
 * name for self-closing tags like "<r/>" (name becomes "r/"). We allow a single
 * trailing '/' for validation purposes.
 */
static bool xml_name_is_ascii_letter(uint8_t c) {
	return (c >= (uint8_t)'A' && c <= (uint8_t)'Z') || (c >= (uint8_t)'a' && c <= (uint8_t)'z');
}

static bool xml_name_is_start_char(uint8_t c) {
	return xml_name_is_ascii_letter(c) || c == (uint8_t)'_' || c == (uint8_t)':';
}

static bool xml_name_is_char(uint8_t c) {
	return xml_name_is_start_char(c)
		|| (c >= (uint8_t)'0' && c <= (uint8_t)'9')
		|| c == (uint8_t)'-'
		|| c == (uint8_t)'.';
}

static bool xml_validate_tag_name(struct xml_parser* parser, struct xml_string* name, char const* context) {
	if (!name || !name->buffer || name->length == 0) {
		xml_parser_error(parser, NO_CHARACTER, context);
		return false;
	}

	size_t len = name->length;
	/* Allow trailing '/' for self-closing tags stored as "tag/". */
	if (len > 0 && name->buffer[len - 1] == (uint8_t)'/') {
		len--;
	}
	if (len == 0) {
		xml_parser_error(parser, NO_CHARACTER, context);
		return false;
	}

	if (!xml_name_is_start_char(name->buffer[0])) {
		xml_parser_error(parser, NO_CHARACTER, context);
		return false;
	}
	for (size_t i = 1; i < len; ++i) {
		if (!xml_name_is_char(name->buffer[i])) {
			xml_parser_error(parser, NO_CHARACTER, context);
			return false;
		}
	}
	return true;
}

/**
 * [PRIVATE]
 *
 * Finds and creates all attributes on the given node.
 * Uses quote-aware parsing so attribute values may contain spaces (XML-compliant; docs/issues.md #33).
 *
 * @author Blake Felt
 * @see https://github.com/Molorius
 */
static struct xml_attribute** xml_find_attributes(struct xml_parser* parser, struct xml_string* tag_open) {
	xml_parser_info(parser, "find_attributes");
	char* tmp;
	char* rest = NULL;
	char* token;
	const unsigned char* start_name;
	const unsigned char* start_content;
	size_t name_len;
	size_t content_len;
	size_t old_elements;
	size_t new_elements;
	struct xml_attribute* new_attribute;
	struct xml_attribute** attributes;
	const char* delim;

	attributes = calloc(1, sizeof(struct xml_attribute*));
	attributes[0] = 0;

	tmp = (char*) xml_string_clone(tag_open);
	delim = XML_ATTRIBUTE_TOKEN_DELIMITERS();

	/* First token is the tag name; remainder is attributes (docs/issues.md #38, #39). */
	token = xml_strtok_r(tmp, delim, &rest);
	if (token == NULL) {
		goto cleanup;
	}
	tag_open->length = strlen(token);

	/* Parse remainder with quote-aware scan: name="value" or name='value' (value may contain spaces). */
	while (rest != NULL) {
		char* ptr;
		char* name_start;
		char* name_end;
		char* eq;
		char quote;
		char* value_start;
		char* value_end;
		ptrdiff_t offset_name;
		ptrdiff_t offset_content;

		ptr = skip_delim(rest, delim);
		if (*ptr == '\0') {
			break;
		}

		eq = strchr(ptr, '=');
		if (eq == NULL || eq == ptr) {
			rest = eq ? eq + 1 : ptr + strlen(ptr);
			continue;
		}

		name_start = ptr;
		name_end = eq;
		while (name_end > name_start && strchr(delim, (unsigned char)name_end[-1]) != NULL) {
			name_end--;
		}
		name_len = (size_t)(name_end - name_start);
		if (name_len == 0) {
			rest = eq + 1;
			continue;
		}

		ptr = eq + 1;
		ptr = skip_delim(ptr, delim);
		if (*ptr != '"' && *ptr != '\'') {
			rest = ptr + 1;
			continue;
		}
		quote = *ptr;
		value_start = ptr + 1;
		value_end = strchr(value_start, (int)(unsigned char)quote);
		if (value_end == NULL) {
			rest = value_start + strlen(value_start);
			continue;
		}
		content_len = (size_t)(value_end - value_start);

		offset_name = name_start - tmp;
		offset_content = value_start - tmp;
		start_name = &tag_open->buffer[offset_name];
		start_content = &tag_open->buffer[offset_content];

		new_attribute = malloc(sizeof(struct xml_attribute));
		if (!new_attribute) {
			goto cleanup_attributes;
		}
		new_attribute->name = malloc(sizeof(struct xml_string));
		new_attribute->content = malloc(sizeof(struct xml_string));
		if (!new_attribute->name || !new_attribute->content) {
			if (new_attribute->name) free(new_attribute->name);
			if (new_attribute->content) free(new_attribute->content);
			free(new_attribute);
			goto cleanup_attributes;
		}
		new_attribute->name->buffer = (unsigned char*)start_name;
		new_attribute->name->length = name_len;
		new_attribute->name->buffer_owned = false;
		new_attribute->content->buffer = (unsigned char*)start_content;
		new_attribute->content->length = content_len;
		new_attribute->content->buffer_owned = false;

		old_elements = get_zero_terminated_array_attributes(attributes);
		/* Reject duplicate attribute names on the same element (XML well-formedness). */
		for (size_t i = 0; i < old_elements; ++i) {
			struct xml_string* existing = attributes[i]->name;
			if (existing
			    && existing->length == name_len
			    && memcmp(existing->buffer, start_name, name_len) == 0) {
				xml_parser_error(parser, NO_CHARACTER, "xml_find_attributes::duplicate attribute name");
				xml_attribute_free(new_attribute);
				goto cleanup_attributes;
			}
		}
		new_elements = old_elements + 1;
		{
			struct xml_attribute** new_attributes = realloc(attributes, (new_elements + 1) * sizeof(struct xml_attribute*));
			if (!new_attributes) {
				xml_attribute_free(new_attribute);
				goto cleanup_attributes;
			}
			attributes = new_attributes;
		}
		attributes[new_elements - 1] = new_attribute;
		attributes[new_elements] = 0;

		rest = value_end + 1;
	}

	goto cleanup;

cleanup_attributes: {
		struct xml_attribute** at = attributes;
		while (at && *at) {
			xml_attribute_free(*at);
			++at;
		}
		free(attributes);
		attributes = NULL;
	}
cleanup:
	free(tmp);
	return attributes;
}


/**
 * [PRIVATE]
 *
 * Parses the name out of an XML tag's ending
 *
 * \code
 * tag_name>
 * \endcode
 */
static struct xml_string* xml_parse_tag_end(struct xml_parser* parser) {
	xml_parser_info(parser, "tag_end");
	size_t start = parser->position;
	size_t length = 0;

	/* Parse until `>' or a whitespace is reached. Use raw character at
	 * parser->position so we stop at the first space; then we expect `>'
	 * and fail for tags with attributes (see docs/testable_issues_priority.md §2).
	 */
	while (start + length < parser->length) {
		uint8_t current = parser->buffer[parser->position];

		if (('>' == current) || isspace((unsigned char)current)) {
			break;
		} else {
			xml_parser_consume(parser, 1);
			length++;
		}
	}

	/* Consume `>'
	 */
	if (parser->position >= parser->length || '>' != parser->buffer[parser->position]) {
		xml_parser_error(parser, CURRENT_CHARACTER, "xml_parse_tag_end::expected tag end");
		return 0;
	}
	xml_parser_consume(parser, 1);

	/* Return parsed tag name
	 */
	struct xml_string* name = malloc(sizeof(struct xml_string));
	name->buffer = &parser->buffer[start];
	name->length = length;
	name->buffer_owned = false;
	return name;
}


/**
 * [PRIVATE]
 *
 * Parses the full content of an opening tag (name and optional attributes) up to
 * and including the closing `>'. Used so that xml_find_attributes can parse
 * attributes from the same buffer.
 *
 * \code
 * tag_name attr="value">
 * \endcode
 */
static struct xml_string* xml_parse_open_tag_content(struct xml_parser* parser) {
	xml_parser_info(parser, "open_tag_content");
	size_t start = parser->position;

	while (parser->position < parser->length && '>' != parser->buffer[parser->position]) {
		xml_parser_consume(parser, 1);
	}

	if (parser->position >= parser->length || '>' != parser->buffer[parser->position]) {
		xml_parser_error(parser, CURRENT_CHARACTER, "xml_parse_open_tag_content::expected tag end");
		return 0;
	}
	xml_parser_consume(parser, 1);

	struct xml_string* content = malloc(sizeof(struct xml_string));
	content->buffer = &parser->buffer[start];
	content->length = (parser->position - 1) - start;
	content->buffer_owned = false;
	return content;
}


/**
 * [PRIVATE]
 *
 * Parses an opening XML tag (name and optional attributes).
 *
 * \code
 * <tag_name>
 * <tag_name attr="value">
 * \endcode
 */
static struct xml_string* xml_parse_tag_open(struct xml_parser* parser) {
	xml_parser_info(parser, "tag_open");
	xml_skip_whitespace(parser);
	while (xml_skip_comment(parser) || xml_skip_processing_instruction(parser))
		xml_skip_whitespace(parser);

	/* Consume `<'
	 */
	if ('<' != xml_parser_peek(parser, CURRENT_CHARACTER)) {
		xml_parser_error(parser, CURRENT_CHARACTER, "xml_parse_tag_open::expected opening tag");
		return 0;
	}
	xml_parser_consume(parser, 1);

	/* Consume full tag content (name and optional attributes) up to `>'
	 */
	return xml_parse_open_tag_content(parser);
}


/**
 * [PRIVATE]
 *
 * Parses a closing XML tag without attributes
 *
 * \code
 * </tag_name>
 * \endcode
 */
static struct xml_string* xml_parse_tag_close(struct xml_parser* parser) {
	xml_parser_info(parser, "tag_close");
	xml_skip_whitespace(parser);
	while (xml_skip_comment(parser) || xml_skip_processing_instruction(parser))
		xml_skip_whitespace(parser);

	/* Consume `</'
	 */
	if (		('<' != xml_parser_peek(parser, CURRENT_CHARACTER))
		||	('/' != xml_parser_peek(parser, NEXT_CHARACTER))) {

		if ('<' != xml_parser_peek(parser, CURRENT_CHARACTER)) {
			xml_parser_error(parser, CURRENT_CHARACTER, "xml_parse_tag_close::expected closing tag `<'");
		}
		if ('/' != xml_parser_peek(parser, NEXT_CHARACTER)) {
			xml_parser_error(parser, NEXT_CHARACTER, "xml_parse_tag_close::expected closing tag `/'");
		}

		return 0;
	}
	xml_parser_consume(parser, 2);

	/* Consume tag name
	 */
	return xml_parse_tag_end(parser);
}


/**
 * [PRIVATE]
 *
 * Parses a tag's content
 *
 * \code
 *     this is
 *   a
 *       tag {} content
 * \endcode
 *
 * CDATA sections are handled in the caller (xml_parse_node). (docs/issues.md #40)
 */
static struct xml_string* xml_parse_content(struct xml_parser* parser) {
	xml_parser_info(parser, "content");

	/* Whitespace will be ignored
	 */
	xml_skip_whitespace(parser);

	size_t start = parser->position;
	size_t length = 0;

	/* Consume until `<' is reached
	 */
	while (start + length < parser->length) {
		uint8_t current = xml_parser_peek(parser, CURRENT_CHARACTER);

		if ('<' == current) {
			break;
		} else {
			xml_parser_consume(parser, 1);
			length++;
		}
	}

	/* Next character must be an `<' or we have reached end of file
	 */
	if ('<' != xml_parser_peek(parser, CURRENT_CHARACTER)) {
		xml_parser_error(parser, CURRENT_CHARACTER, "xml_parse_content::expected <");
		return 0;
	}

	/* Ignore tailing whitespace
	 */
	while ((length > 0) && isspace(parser->buffer[start + length - 1])) {
		length--;
	}

	/* Return text
	 */
	struct xml_string* content = malloc(sizeof(struct xml_string));
	content->buffer = &parser->buffer[start];
	content->length = length;
	content->buffer_owned = false;
	return content;
}


/**
 * [PRIVATE]
 *
 * Appends a segment to node content (for mixed content and CDATA). If existing
 * is NULL and seg_len is 0, returns NULL. If existing is NULL, returns a new
 * xml_string pointing at seg_buf (not owned). Otherwise concatenates and may
 * allocate (buffer_owned set when concatenating). (docs/issues.md #40)
 */
static struct xml_string* content_append(struct xml_parser* parser,
	struct xml_string* existing, uint8_t const* seg_buf, size_t seg_len) {
	(void)parser;
	if (!seg_buf && seg_len == 0)
		return existing;
	if (!existing) {
		struct xml_string* s = malloc(sizeof(struct xml_string));
		if (!s)
			return NULL;
		s->buffer = seg_buf;
		s->length = seg_len;
		s->buffer_owned = false;
		return s;
	}
	/* Concatenate: allocate new buffer, copy both, return new string (owned). */
	size_t total = existing->length + seg_len;
	uint8_t* copy = malloc(total);
	if (!copy)
		return existing;
	memcpy(copy, existing->buffer, existing->length);
	memcpy(copy + existing->length, seg_buf, seg_len);
	xml_string_free(existing);
	struct xml_string* s = malloc(sizeof(struct xml_string));
	if (!s) {
		free(copy);
		return NULL;
	}
	s->buffer = copy;
	s->length = total;
	s->buffer_owned = true;
	return s;
}


/**
 * [PRIVATE]
 * 
 * Parses an XML fragment node
 *
 * Without children:
 * \code
 * <Node>Text</Node>
 * \endcode
 *
 * With children:
 * \code
 * <Parent>
 *     <Child>Text</Child>
 *     <Child>Text</Child>
 *     <Test>Content</Test>
 * </Parent>
 * \endcode
 */
static struct xml_node* xml_parse_node(struct xml_parser* parser) {
	xml_parser_info(parser, "node");

	/* Setup variables
	 */
	struct xml_string* tag_open = 0;
	struct xml_string* tag_close = 0;
	struct xml_string* content = 0;

	size_t original_length;
	struct xml_attribute** attributes = 0;

	struct xml_node** children = calloc(1, sizeof(struct xml_node*));
	children[0] = 0;


	/* Parse open tag
	 */
	tag_open = xml_parse_tag_open(parser);
	if (!tag_open) {
		xml_parser_error(parser, NO_CHARACTER, "xml_parse_node::tag_open");
		goto exit_failure;
	}

	original_length = tag_open->length;
	attributes = xml_find_attributes(parser, tag_open);
	if (!attributes) {
		xml_parser_error(parser, NO_CHARACTER, "xml_parse_node::attributes");
		goto exit_failure;
	}

	/* Enforce XML-ish Name production on tag names (docs/issues.md: stricter tag names). */
	if (!xml_validate_tag_name(parser, tag_open, "xml_parse_node::invalid tag name")) {
		goto exit_failure;
	}

	/* If tag ends with `/' it's self closing, skip content lookup */
	if (tag_open->length > 0 && '/' == tag_open->buffer[original_length - 1]) {
		/* Drop `/'
		 */
		goto node_creation;
	}

	/* Parse content and/or children: text, CDATA, and child elements (docs/issues.md #40)
	 */
	for (;;) {
		xml_skip_whitespace(parser);
		while (xml_skip_comment(parser) || xml_skip_processing_instruction(parser))
			xml_skip_whitespace(parser);
		if (parser->position >= parser->length)
			break;
		/* Closing tag */
		if (parser->buffer[parser->position] == '<'
		    && parser->position + 1 < parser->length
		    && parser->buffer[parser->position + 1] == '/')
			break;
		/* CDATA section */
		if (parser->position + 9 <= parser->length
		    && memcmp((char const*)&parser->buffer[parser->position], "<![CDATA[", 9) == 0) {
			struct xml_string* cdata = xml_parse_cdata_section(parser);
			if (!cdata) {
				xml_parser_error(parser, 0, "xml_parse_node::cdata");
				goto exit_failure;
			}
			{
				size_t cdata_len = cdata->length;
				content = content_append(parser, content, cdata->buffer, cdata_len);
				xml_string_free(cdata);
				if (!content && cdata_len > 0) {
					xml_parser_error(parser, 0, "xml_parse_node::content_append");
					goto exit_failure;
				}
			}
			continue;
		}
		/* Child element (starts with '<' and a name character) */
		if (parser->buffer[parser->position] == '<') {
			struct xml_node* child = xml_parse_node(parser);
			if (!child) {
				xml_parser_error(parser, NEXT_CHARACTER, "xml_parse_node::child");
				goto exit_failure;
			}
			size_t old_elements = get_zero_terminated_array_nodes(children);
			size_t new_elements = old_elements + 1;
			struct xml_node** new_children = realloc(children, (new_elements + 1) * sizeof(struct xml_node*));
			if (!new_children) {
				xml_node_free(child);
				xml_parser_error(parser, NEXT_CHARACTER, "xml_parse_node::children");
				goto exit_failure;
			}
			children = new_children;
			children[new_elements - 1] = child;
			children[new_elements] = 0;
			continue;
		}
		/* Text content (does not start with '<') */
		{
			struct xml_string* text = xml_parse_content(parser);
			if (!text) {
				xml_parser_error(parser, 0, "xml_parse_node::content");
				goto exit_failure;
			}
			{
				size_t text_len = text->length;
				content = content_append(parser, content, text->buffer, text_len);
				xml_string_free(text);
				if (!content && text_len > 0) {
					xml_parser_error(parser, 0, "xml_parse_node::content_append");
					goto exit_failure;
				}
			}
		}
	}


	/* Parse close tag
	 */
	tag_close = xml_parse_tag_close(parser);
	if (!tag_close) {
		xml_parser_error(parser, NO_CHARACTER, "xml_parse_node::tag_close");
		goto exit_failure;
	}

	if (!xml_validate_tag_name(parser, tag_close, "xml_parse_node::invalid closing tag name")) {
		goto exit_failure;
	}


	/* Close tag has to match open tag
	 */
	if (!xml_string_equals(tag_open, tag_close)) {
		xml_parser_error(parser, NO_CHARACTER, "xml_parse_node::tag mismatch");
		goto exit_failure;
	}


	/* Return parsed node
	 */
	xml_string_free(tag_close);

node_creation:;
	struct xml_node* node = malloc(sizeof(struct xml_node));
	node->name = tag_open;
	node->content = content;
	node->attributes = attributes;
	node->children = children;
	return node;


	/* A failure occurred, so free all allocated resources
	 */
exit_failure:
	if (tag_open) {
		xml_string_free(tag_open);
	}
	if (tag_close) {
		xml_string_free(tag_close);
	}
	if (content) {
		xml_string_free(content);
	}
	if (attributes) {
		struct xml_attribute** at = attributes;
		while (*at) {
			xml_attribute_free(*at);
			++at;
		}
		free(attributes);
	}

	struct xml_node** it = children;
	while (*it) {
		xml_node_free(*it);
		++it;
	}
	free(children);

	return 0;
}


/**
 * [PUBLIC API]
 */
struct xml_document* xml_parse_document(uint8_t* buffer, size_t length) {

	/* Initialize parser
	 */
	struct xml_parser parser = {
		.buffer = buffer,
		.position = 0,
		.length = length
	};

	/* An empty buffer can never contain a valid document
	 */
	if (!length) {
		xml_parser_error(&parser, NO_CHARACTER, "xml_parse_document::length equals zero");
		return 0;
	}

	/* Parse the root node
	 */
	struct xml_node* root = xml_parse_node(&parser);
	if (!root) {
		xml_parser_error(&parser, NO_CHARACTER, "xml_parse_document::parsing document failed");
		return 0;
	}

	/* Return parsed document
	 */
	struct xml_document* document = malloc(sizeof(struct xml_document));
	document->buffer.buffer = buffer;
	document->buffer.length = length;
	document->root = root;

	return document;
}


/**
 * [PUBLIC API]
 */
struct xml_document* xml_open_document(FILE* source) {

	/* Prepare buffer
	 */
	size_t const read_chunk = 1; // TODO 4096;

	size_t document_length = 0;
	size_t buffer_size = 1;	// TODO 4069
	uint8_t* buffer = malloc(buffer_size * sizeof(uint8_t));

	/* Read whole file into buffer (loop on fread result to avoid feof extra iteration)
	 */
	size_t read;
	for (;;) {
		if (buffer_size - document_length < read_chunk) {
			uint8_t* new_buffer = realloc(buffer, buffer_size + 2 * read_chunk);
			if (!new_buffer) {
				fclose(source);
				free(buffer);
				return 0;
			}
			buffer = new_buffer;
			buffer_size += 2 * read_chunk;
		}
		read = fread(&buffer[document_length], sizeof(uint8_t), read_chunk, source);
		document_length += read;
		if (read != read_chunk)
			break;
	}

	if (ferror(source)) {
		fclose(source);
		free(buffer);
		return 0;
	}
	if (fclose(source) != 0) {
		free(buffer);
		return 0;
	}

	/* Try to parse buffer
	 */
	struct xml_document* document = xml_parse_document(buffer, document_length);

	if (!document) {
		free(buffer);
		return 0;
	}
	return document;
}


/**
 * [PUBLIC API]
 */
void xml_document_free(struct xml_document* document, bool free_buffer) {
	if (!document) {
		return;
	}
	xml_node_free(document->root);

	if (free_buffer) {
		free(document->buffer.buffer);
	}
	free(document);
}


/**
 * [PUBLIC API]
 */
struct xml_node* xml_document_root(struct xml_document* document) {
	if (!document) {
		return 0;
	}
	return document->root;
}


/**
 * [PUBLIC API]
 */
size_t xml_document_buffer_length(struct xml_document* document) {
	if (!document) {
		return 0;
	}
	return document->buffer.length;
}


/**
 * [PUBLIC API]
 */
struct xml_string* xml_node_name(struct xml_node* node) {
	if (!node) {
		return 0;
	}
	return node->name;
}


/**
 * [PUBLIC API]
 */
struct xml_string* xml_node_content(struct xml_node* node) {
	if (!node) {
		return 0;
	}
	return node->content;
}


/**
 * [PUBLIC API]
 *
 * @warning O(n)
 */
size_t xml_node_children(struct xml_node* node) {
	if (!node || !node->children) {
		return 0;
	}
	return get_zero_terminated_array_nodes(node->children);
}


/**
 * [PUBLIC API]
 */
struct xml_node* xml_node_child(struct xml_node* node, size_t child) {
	if (!node || !node->children) {
		return 0;
	}
	if (child >= xml_node_children(node)) {
		return 0;
	}

	return node->children[child];
}


/**
 * [PUBLIC API]
 */
size_t xml_node_attributes(struct xml_node* node) {
	if (!node || !node->attributes) {
		return 0;
	}
	return get_zero_terminated_array_attributes(node->attributes);
}


/**
 * [PUBLIC API]
 */
struct xml_string* xml_node_attribute_name(struct xml_node* node, size_t attribute) {
	if (!node || !node->attributes) {
		return 0;
	}
	if(attribute >= xml_node_attributes(node)) {
		return 0;
	}

	return node->attributes[attribute]->name;
}


/**
 * [PUBLIC API]
 */
struct xml_string* xml_node_attribute_content(struct xml_node* node, size_t attribute) {
	if (!node || !node->attributes) {
		return 0;
	}
	if(attribute >= xml_node_attributes(node)) {
		return 0;
	}

	return node->attributes[attribute]->content;
}


/**
 * [PUBLIC API]
 */
struct xml_node* xml_easy_child(struct xml_node* node, uint8_t const* child_name, ...) {

	if (!node) {
		return 0;
	}

	/* Find children, one by one
	 */
	struct xml_node* current = node;

	va_list arguments;
	va_start(arguments, child_name);


	/* Descent to current.child
	 */
	while (child_name) {

		/* Convert child_name to xml_string for easy comparison
		 */
		struct xml_string cn = {
			.buffer = child_name,
			.length = strlen((char const*)child_name)
		};

		/* Interate through all children
		 */
		struct xml_node* next = 0;

		size_t i = 0; for (; i < xml_node_children(current); ++i) {
			struct xml_node* child = xml_node_child(current, i);

			if (xml_string_equals(xml_node_name(child), &cn)) {
				if (!next) {
					next = child;

				/* Two children with the same name
				 */
				} else {
					va_end(arguments);
					return 0;
				}
			}
		}

		/* No child with that name found
		 */
		if (!next) {
			va_end(arguments);
			return 0;
		}
		current = next;		
		
		/* Find name of next child
		 */
		child_name = va_arg(arguments, uint8_t const*);
	}
	va_end(arguments);


	/* Return current element
	 */
	return current;
}


/**
 * [PUBLIC API]
 */
uint8_t* xml_node_name_c_string(struct xml_node* node) {
	if (!node) {
		return 0;
	}

	return xml_string_clone(xml_node_name(node));
}


/**
 * [PUBLIC API]
 */
uint8_t* xml_node_content_c_string(struct xml_node* node) {
	if (!node) {
		return 0;
	}

	return xml_string_clone(xml_node_content(node));
}


/**
 * [PUBLIC API]
 */
uint8_t* xml_node_attribute_name_c_string(struct xml_node* node, size_t attribute) {
	struct xml_string* s = xml_node_attribute_name(node, attribute);
	return s ? xml_string_clone(s) : 0;
}


/**
 * [PUBLIC API]
 */
uint8_t* xml_node_attribute_content_c_string(struct xml_node* node, size_t attribute) {
	struct xml_string* s = xml_node_attribute_content(node, attribute);
	return s ? xml_string_clone(s) : 0;
}


/**
 * [PUBLIC API]
 */
size_t xml_string_length(struct xml_string* string) {
	if (!string) {
		return 0;
	}
	return string->length;
}


/**
 * [PUBLIC API]
 */
void xml_string_copy(struct xml_string* string, uint8_t* buffer, size_t length) {
	if (!string) {
		return;
	}

	length = MIN(length, string->length);

	memcpy(buffer, string->buffer, length);
}


/**
 * [PUBLIC API]
 */
bool xml_string_equals(struct xml_string* a, struct xml_string* b) {
	if (!a || !b) {
		return false;
	}
	if (a->length != b->length) {
		return false;
	}
	return memcmp(a->buffer, b->buffer, a->length) == 0;
}


/**
 * [PUBLIC API]
 */
bool xml_string_equals_cstr(struct xml_string* string, uint8_t const* cstr) {
	if (!string) {
		return false;
	}
	size_t cstr_len = (cstr == NULL) ? 0 : strlen((char const*)cstr);
	if (string->length != cstr_len) {
		return false;
	}
	if (cstr_len == 0) {
		return true;
	}
	return memcmp(string->buffer, cstr, string->length) == 0;
}

