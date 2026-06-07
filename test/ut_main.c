/*
 * Feature unit test runner — aggregates all ut_* test suites.
 *
 * Label: unit
 * Run:   ctest -L unit
 *        ./xml-test-features
 */
#include <stdlib.h>
#include <string.h>
#include <cmocka.h>
#include "ut_runner.h"


int main(void) {
	const struct CMUnitTest *t1, *t2, *t3, *t4, *t5, *t6, *t7, *t8, *t9;
	size_t n1, n2, n3, n4, n5, n6, n7, n8, n9;

	get_ut_parse_document_tests(&t1, &n1);
	get_ut_open_document_tests(&t2, &n2);
	get_ut_document_root_tests(&t3, &n3);
	get_ut_node_content_tests(&t4, &n4);
	get_ut_node_attributes_tests(&t5, &n5);
	get_ut_easy_child_tests(&t6, &n6);
	get_ut_c_string_api_tests(&t7, &n7);
	get_ut_string_api_tests(&t8, &n8);
	get_ut_integration_tests(&t9, &n9);

	size_t total = n1 + n2 + n3 + n4 + n5 + n6 + n7 + n8 + n9;
	struct CMUnitTest *all = malloc(total * sizeof(struct CMUnitTest));
	if (!all) { return 1; }

	size_t off = 0;
	memcpy(all + off, t1, n1 * sizeof(struct CMUnitTest)); off += n1;
	memcpy(all + off, t2, n2 * sizeof(struct CMUnitTest)); off += n2;
	memcpy(all + off, t3, n3 * sizeof(struct CMUnitTest)); off += n3;
	memcpy(all + off, t4, n4 * sizeof(struct CMUnitTest)); off += n4;
	memcpy(all + off, t5, n5 * sizeof(struct CMUnitTest)); off += n5;
	memcpy(all + off, t6, n6 * sizeof(struct CMUnitTest)); off += n6;
	memcpy(all + off, t7, n7 * sizeof(struct CMUnitTest)); off += n7;
	memcpy(all + off, t8, n8 * sizeof(struct CMUnitTest)); off += n8;
	memcpy(all + off, t9, n9 * sizeof(struct CMUnitTest));

	int ret = _cmocka_run_group_tests("feature-unit", all, total, NULL, NULL);
	free(all);
	return ret;
}
