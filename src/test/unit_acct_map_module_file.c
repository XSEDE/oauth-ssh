/*
 * Copyright The University of Chicago
 * All Rights Reserved.
 */

/*
 * System includes.
 */
#include <stdio.h>
#include <errno.h>

/*
 * Local includes.
 */
#include "unit_test.h"
#include "../acct_map_module_file.c"

/*******************************************
 *              MOCKS
 *******************************************/
#include "mock_config.c"

/*******************************************
 *              TESTS
 *******************************************/

#define PATH "path"
#define ID "id"
#define ACCOUNT "account"

void
test_module_is_defined(void ** state)
{
	assert_non_null(acct_map_module_map_file.initialize);
	assert_non_null(acct_map_module_map_file.lookup);
	assert_non_null(acct_map_module_map_file.finalize);
}

void
test_eperm_on_load(void ** state)
{
	expect_string(config_load, path, PATH);
	will_return(config_load, -EPERM);

	void * handle;
	int retval = acct_map_module_map_file.initialize(&handle, PATH);
	assert_int_equal(retval, -EPERM);
}

void
test_successful_lookup(void ** state)
{
	// initialize()
	expect_string(config_load, path, PATH);
	will_return(config_load, 0);
	void * handle;
	acct_map_module_map_file.initialize(&handle, PATH);

	// lookup()
	expect_string(config_get_value, key, ID);
	will_return(config_get_value, ACCOUNT);
	char * account = acct_map_module_map_file.lookup(handle, ID);
	assert_string_equal(account, ACCOUNT);
	free(account);

	// finalize()
	acct_map_module_map_file.finalize(handle);
}

void
test_failed_lookup(void ** state)
{
	// initialize()
	expect_string(config_load, path, PATH);
	will_return(config_load, 0);
	void * handle;
	acct_map_module_map_file.initialize(&handle, PATH);

	// lookup()
	expect_string(config_get_value, key, ID);
	will_return(config_get_value, NULL);
	char * account = acct_map_module_map_file.lookup(handle, ID);
	assert_null(account);

	// finalize()
	acct_map_module_map_file.finalize(handle);
}

/*******************************************
 *              FIXTURES
 *******************************************/

int
main()
{
	const struct CMUnitTest tests[] = {
		{"check module definition", test_module_is_defined},
		{"propogate config error", test_eperm_on_load},
		{"successful lookup", test_successful_lookup},
		{"failed lookup", test_failed_lookup},
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}

