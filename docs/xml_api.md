# xml.c API reference

Generated from Doxygen comments in `src/xml.h`. Regenerate with:
`cmake --build build --target api_docs`.

---

## xml_parse_document

**struct xml_document *** `xml_parse_document`(uint8_t *buffer, size_t length)

Tries to parse the XML fragment in buffer

- **buffer** — Chunk to parse
- **length** — Size of the buffer

**Warning:** buffer will be referenced by the document, you may not free it until you free the xml_document

**Warning:** You have to call xml_document_free after you finished using the document

**Returns:** The parsed xml fragment iff parsing was successful, 0 otherwise


## xml_open_document

**struct xml_document *** `xml_open_document`(FILE *source)

Tries to read an XML document from disk

- **source** — File that will be read into an xml document. Will be closed

**Warning:** You have to call xml_document_free with free_buffer = true after you finished using the document

**Returns:** The parsed xml fragment iff parsing was successful, 0 otherwise


## xml_document_free

**void** `xml_document_free`(struct xml_document *document, bool free_buffer)

Frees all resources associated with the document. All xml_node and xml_string references obtained through the document will be invalidated

- **document** — xml_document to free
- **free_buffer** — iff true the internal buffer supplied via xml_parse_document will be freed with the free system call


## xml_document_root

**struct xml_node *** `xml_document_root`(struct xml_document *document)

- **document** — The document

**Returns:** xml_node representing the document root


## xml_document_buffer_length

**size_t** `xml_document_buffer_length`(struct xml_document *document)

- **document** — The document

**Returns:** Length in bytes of the buffer that was parsed to create the document, or 0 if document is NULL. Useful for validation (e.g. comparing to file size when the document was opened with xml_open_document).


## xml_node_name

**struct xml_string *** `xml_node_name`(struct xml_node *node)

- **node** — The node

**Returns:** The xml_node's tag name


## xml_node_content

**struct xml_string *** `xml_node_content`(struct xml_node *node)

- **node** — The node

**Returns:** The xml_node's string content (if available, otherwise NULL)


## xml_node_children

**size_t** `xml_node_children`(struct xml_node *node)

- **node** — The node

**Returns:** Number of child nodes

**Warning:** O(n)


## xml_node_child

**struct xml_node *** `xml_node_child`(struct xml_node *node, size_t child)

- **node** — The node
- **child** — Zero-based index of the child

**Returns:** The n-th child or 0 if out of range


## xml_node_attributes

**size_t** `xml_node_attributes`(struct xml_node *node)

- **node** — The node

**Returns:** Number of attribute nodes


## xml_node_attribute_name

**struct xml_string *** `xml_node_attribute_name`(struct xml_node *node, size_t attribute)

- **node** — The node
- **attribute** — Zero-based index of the attribute

**Returns:** the n-th attribute name or 0 if out of range


## xml_node_attribute_content

**struct xml_string *** `xml_node_attribute_content`(struct xml_node *node, size_t attribute)

- **node** — The node
- **attribute** — Zero-based index of the attribute

**Returns:** the n-th attribute content or 0 if out of range


## xml_easy_child

**struct xml_node *** `xml_easy_child`(struct xml_node *node, uint8_t const *child_name,...)

- **node** — The node from which to start
- **child_name** — First path segment (tag name); further segments follow as variadic arguments; list must end with NULL (or 0)

**Returns:** The node described by the path or 0 if child cannot be found

**Warning:** Each element on the way must be unique


## xml_node_name_c_string

**uint8_t *** `xml_node_name_c_string`(struct xml_node *node)

- **node** — The node

**Returns:** 0-terminated C string copy of node name (caller must free)


## xml_node_content_c_string

**uint8_t *** `xml_node_content_c_string`(struct xml_node *node)

- **node** — The node

**Returns:** 0-terminated C string copy of node content (caller must free)


## xml_string_length

**size_t** `xml_string_length`(struct xml_string *string)

- **string** — The string

**Returns:** Length of the string


## xml_string_copy

**void** `xml_string_copy`(struct xml_string *string, uint8_t *buffer, size_t length)

Copies the string into the supplied buffer

- **string** — The string to copy
- **buffer** — Destination buffer
- **length** — Size of the buffer (at most this many bytes are written)

**Warning:** String will not be 0-terminated

**Warning:** Will write at most length bytes, even if the string is longer

