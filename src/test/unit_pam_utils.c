/*
 * Copyright The University of Chicago
 * All Rights Reserved.
 */

/*
 * Local includes.
 */
#include "unit_test.h"
#include "../pam_utils.h"

/*******************************************
 *              MOCKS
 *******************************************/

/*******************************************
 *              TESTS
 *******************************************/

void
test_get_array_len(void ** state)
{
	assert_int_equal(get_array_len(NULL), 0);
	assert_int_equal(get_array_len((void *[]){NULL}), 0);
	assert_int_equal(get_array_len((void *[]){"Hello", NULL}), 1);
}

/*******************************************
 *              FIXTURES
 *******************************************/

int
main()
{
	const struct CMUnitTest tests[] = {
		{"get_array_len", test_get_array_len},
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}
