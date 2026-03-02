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
#ifndef HEADER_XML
#define HEADER_XML


/**
 * Includes
 *
 * This header is self-contained: it includes <stdbool.h>, <stdint.h>, <stdio.h>,
 * and <string.h>. You need only #include <xml.h> to use the API (uint8_t, size_t,
 * bool, FILE, and the declared functions).
 *
 * Encoding: The library assumes UTF-8 for all input and output. Buffers passed to
 * xml_parse_document, and all tag names, text content, and attribute values exposed
 * via the API, are UTF-8 byte sequences. Character references are expanded to UTF-8.
 * No encoding conversion or validation is performed.
 */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Opaque structure holding the parsed xml document
 */
struct xml_document;
struct xml_node;
struct xml_attribute;

/**
 * Internal character sequence representation (UTF-8).
 * See the header "Encoding" note: all string data is assumed to be UTF-8.
 */
struct xml_string;



/**
 * Tries to parse the XML fragment in buffer
 *
 * @param buffer Chunk to parse (UTF-8; no conversion or validation)
 * @param length Size of the buffer in bytes
 *
 * @warning `buffer` will be referenced by the document, you may not free it
 *     until you free the xml_document
 * @warning You have to call xml_document_free after you finished using the
 *     document
 *
 * @return The parsed xml fragment iff parsing was successful, 0 otherwise
 */
struct xml_document* xml_parse_document(uint8_t* buffer, size_t length);



/**
 * Tries to read an XML document from disk
 *
 * @param source File that will be read into an xml document. Will be closed.
 *     File content is assumed to be UTF-8.
 *
 * @warning You have to call xml_document_free with free_buffer = true after you
 *     finished using the document
 *
 * @return The parsed xml fragment iff parsing was successful, 0 otherwise
 */
struct xml_document* xml_open_document(FILE* source);



/**
 * Frees all resources associated with the document. All xml_node and xml_string
 * references obtained through the document will be invalidated
 *
 * @param document xml_document to free
 * @param free_buffer iff true the internal buffer supplied via xml_parse_document
 *     will be freed with the `free` system call
 */
void xml_document_free(struct xml_document* document, bool free_buffer);


/**
 * @param document The document
 * @return xml_node representing the document root
 */
struct xml_node* xml_document_root(struct xml_document* document);



/**
 * @param document The document
 * @return Length in bytes of the buffer that was parsed to create the document,
 *     or 0 if document is NULL. Useful for validation (e.g. comparing to file
 *     size when the document was opened with xml_open_document).
 */
size_t xml_document_buffer_length(struct xml_document* document);



/**
 * @param node The node
 * @return The xml_node's tag name
 */
struct xml_string* xml_node_name(struct xml_node* node);



/**
 * @param node The node
 * @return The xml_node's string content (if available, otherwise NULL)
 */
struct xml_string* xml_node_content(struct xml_node* node);



/**
 * @param node The node
 * @return Number of child nodes
 */
size_t xml_node_children(struct xml_node* node);



/**
 * @param node The node
 * @param child Zero-based index of the child
 * @return The n-th child or 0 if out of range
 */
struct xml_node* xml_node_child(struct xml_node* node, size_t child);



/**
 * @param node The node
 * @return Number of attribute nodes
 */
size_t xml_node_attributes(struct xml_node* node);



/**
 * @param node The node
 * @param attribute Zero-based index of the attribute
 * @return the n-th attribute name or 0 if out of range
 */
struct xml_string* xml_node_attribute_name(struct xml_node* node, size_t attribute);



/**
 * @param node The node
 * @param attribute Zero-based index of the attribute
 * @return the n-th attribute content or 0 if out of range
 */
struct xml_string* xml_node_attribute_content(struct xml_node* node, size_t attribute);



/**
 * @param node The node from which to start
 * @param child_name First path segment (tag name); further segments follow as variadic
 *     arguments; list must end with NULL (or 0)
 * @return The node described by the path or 0 if child cannot be found
 * @warning Each element on the way must be unique
 */
struct xml_node* xml_easy_child(struct xml_node* node, uint8_t const* child_name, ...);



/**
 * @param node The node
 * @return 0-terminated C string copy of node name (caller must free)
 */
uint8_t* xml_node_name_c_string(struct xml_node* node);



/**
 * @param node The node
 * @return 0-terminated C string copy of node content (caller must free)
 */
uint8_t* xml_node_content_c_string(struct xml_node* node);



/**
 * @param node The node
 * @param attribute Zero-based index of the attribute
 * @return 0-terminated C string copy of the attribute name, or NULL if node is NULL or index out of range (caller must free)
 */
uint8_t* xml_node_attribute_name_c_string(struct xml_node* node, size_t attribute);



/**
 * @param node The node
 * @param attribute Zero-based index of the attribute
 * @return 0-terminated C string copy of the attribute content, or NULL if node is NULL or index out of range (caller must free)
 */
uint8_t* xml_node_attribute_content_c_string(struct xml_node* node, size_t attribute);



/**
 * @param string The string
 * @return Length of the string
 */
size_t xml_string_length(struct xml_string* string);



/**
 * Copies the string into the supplied buffer
 *
 * @param string The string to copy
 * @param buffer Destination buffer
 * @param length Size of the buffer (at most this many bytes are written)
 * @warning String will not be 0-terminated
 * @warning Will write at most length bytes, even if the string is longer
 */
void xml_string_copy(struct xml_string* string, uint8_t* buffer, size_t length);



/**
 * @param a First string (may be NULL)
 * @param b Second string (may be NULL)
 * @return true iff both are non-NULL and have the same length and byte content
 */
bool xml_string_equals(struct xml_string* a, struct xml_string* b);



/**
 * @param string An xml_string (may be NULL)
 * @param cstr 0-terminated C string (NULL is treated as empty string)
 * @return true iff string is non-NULL and its content equals cstr
 */
bool xml_string_equals_cstr(struct xml_string* string, uint8_t const* cstr);

#ifdef __cplusplus
}
#endif

#endif

