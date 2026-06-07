#ifndef UT_RUNNER_H
#define UT_RUNNER_H

#include <stddef.h>
#include <cmocka.h>

void get_ut_parse_document_tests(const struct CMUnitTest **out_tests, size_t *out_count);
void get_ut_open_document_tests(const struct CMUnitTest **out_tests, size_t *out_count);
void get_ut_document_root_tests(const struct CMUnitTest **out_tests, size_t *out_count);
void get_ut_node_content_tests(const struct CMUnitTest **out_tests, size_t *out_count);
void get_ut_node_attributes_tests(const struct CMUnitTest **out_tests, size_t *out_count);
void get_ut_easy_child_tests(const struct CMUnitTest **out_tests, size_t *out_count);
void get_ut_c_string_api_tests(const struct CMUnitTest **out_tests, size_t *out_count);
void get_ut_string_api_tests(const struct CMUnitTest **out_tests, size_t *out_count);
void get_ut_integration_tests(const struct CMUnitTest **out_tests, size_t *out_count);

#endif /* UT_RUNNER_H */
