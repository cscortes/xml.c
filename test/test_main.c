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
#include <stdlib.h>
#include <string.h>
#include <cmocka.h>
#include "test_runner.h"


int main(void) {
	const struct CMUnitTest* t1;
	const struct CMUnitTest* t2;
	const struct CMUnitTest* t3;
	size_t n1, n2, n3;

	get_unit_c_tests(&t1, &n1);
	get_unit_c_null_tests(&t2, &n2);
	get_unit_c_pi_tests(&t3, &n3);

	size_t total = n1 + n2 + n3;
	struct CMUnitTest* all = malloc(total * sizeof(struct CMUnitTest));
	if (!all) {
		return 1;
	}
	memcpy(all, t1, n1 * sizeof(struct CMUnitTest));
	memcpy(all + n1, t2, n2 * sizeof(struct CMUnitTest));
	memcpy(all + n1 + n2, t3, n3 * sizeof(struct CMUnitTest));

	/* Use internal API with explicit count; the macro uses sizeof(array) which is wrong for a pointer. */
	int ret = _cmocka_run_group_tests("all", all, total, NULL, NULL);
	free(all);
	return ret;
}
