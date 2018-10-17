/*
 * Copyright The University of Chicago
 * All Rights Reserved.
 */

/*
 * Local includes.
 */
#include "unit_test.h"
#include "../acct_map_module_idp.c"

/*******************************************
 *              MOCKS
 *******************************************/


/*******************************************
 *              TESTS
 *******************************************/

void
test_check_module(void ** state)
{
	assert_non_null(acct_map_module_idp_suffix.initialize);
	assert_non_null(acct_map_module_idp_suffix.lookup);
	assert_non_null(acct_map_module_idp_suffix.finalize);
}

void
test_initialize(void ** state)
{
	void * handle = NULL;

	assert_int_equal(acct_map_module_idp_suffix.initialize(&handle, ""), 0);
	acct_map_module_idp_suffix.finalize(handle);
}

#define IDP1  "globus.org"
#define IDP2  "globusid.org"
#define USER1 "test_user1"
#define USER2 "test_user2"

void
test_successful_lookup(void ** state)
{
	void * handle = NULL;

	acct_map_module_idp_suffix.initialize(&handle, IDP1);
	char * account = acct_map_module_idp_suffix.lookup(handle, USER1 "@" IDP1);
	assert_string_equal(account, USER1);
	acct_map_module_idp_suffix.finalize(handle);
	test_free(account);
}

void
test_failed_lookup(void ** state)
{
	void * handle = NULL;

	acct_map_module_idp_suffix.initialize(&handle, IDP1);
	char * account = acct_map_module_idp_suffix.lookup(handle, USER2 "@" IDP2);
	assert_true(!account || !account[0]);
	if (account)
		free(account);
	acct_map_module_idp_suffix.finalize(handle);
}

/*******************************************
 *              FIXTURES
 *******************************************/

int
main()
{
	const struct CMUnitTest tests[] = {
		{"check module definition", test_check_module},
		{"successful lookup", test_successful_lookup},
		{"failed lookup", test_failed_lookup},
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}
