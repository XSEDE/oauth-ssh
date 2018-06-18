/*
 * Copyright The University of Chicago
 * All Rights Reserved.
 */

/*
 * System includes.
 */

/*
 * Local includes.
 */
#include "unit_test.h"
#include "../acct_map.c"

/*******************************************
 *              MOCKS
 *******************************************/
#include "mock_acct_map_module.h"

const char * AcctMapModuleMapFile = "MAP FILE";
struct acct_map_module acct_map_module_map_file;

const char * AcctMapModuleIdpSuffix = "IDP SUFFIX";
struct acct_map_module acct_map_module_idp_suffix;


/*******************************************
 *              TESTS
 *******************************************/

static void
test_unknown_module(void ** state)
{
	struct acct_map * acct_map = acct_map_init();
	assert_int_equal(acct_map_add_module(acct_map, "XXXXX", ""), -ENOENT);
	acct_map_free(acct_map);
}

static void
test_error_on_module_load(void ** state)
{
	struct acct_map * acct_map = acct_map_init();

	will_return(initialize, -EINVAL);

	int retval = acct_map_add_module(acct_map, AcctMapModuleMapFile, "");
	assert_int_equal(retval, -EINVAL);
	acct_map_free(acct_map);
}

static void
test_failed_lookup(void ** state)
{
	struct acct_map * acct_map = acct_map_init();

	will_return(initialize, 0);
	int retval = acct_map_add_module(acct_map, AcctMapModuleMapFile, "");
	assert_int_equal(retval, 0);

	expect_string(lookup, id, "username1");
	will_return(lookup, NULL);
	char *  list[]   = {"username1", NULL};
	char ** accounts = acct_map_lookup(acct_map, list);

	assert_true(!accounts || !accounts[0]);
	if (accounts)
		test_free(accounts);
	acct_map_free(acct_map);
}

static void
test_unique_account_returned(void ** state)
{
	struct acct_map * acct_map = acct_map_init();

	will_return(initialize, 0);
	acct_map_add_module(acct_map, AcctMapModuleMapFile, "");

	will_return(initialize, 0);
	acct_map_add_module(acct_map, AcctMapModuleIdpSuffix, "");

	expect_string_count(lookup, id, "first", 2);
	expect_string_count(lookup, id, "second", 2);
	will_return_count(lookup, "account1", 4);

	char * list[] = {"first", "second", NULL};
	char ** accounts = acct_map_lookup(acct_map, list);

	assert_string_equal(accounts[0], "account1");
	assert_null(accounts[1]);

	test_free(accounts[0]);
	test_free(accounts);
	acct_map_free(acct_map);
}


/*******************************************
 *              FIXTURES
 *******************************************/

int
main()
{
	acct_map_module_map_file = acct_map_module_mock;
	acct_map_module_idp_suffix = acct_map_module_mock;

	const struct CMUnitTest tests[] = {
		{"unknown module", test_unknown_module},
		{"error on module load", test_error_on_module_load},
		{"successful lookup", test_failed_lookup},
		{"only unique accounts returned", test_unique_account_returned},
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}

