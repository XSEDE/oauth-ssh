/*
 * Copyright The University of Chicago
 * All Rights Reserved.
 */

/*
 * Local includes.
 */
#include "unit_test.h"
#include "../credentials.c"

/*******************************************
 *              MOCKS
 *******************************************/

/*******************************************
 *              TESTS
 *******************************************/
void
test_init_bearer_creds(void ** state)
{
	struct credentials creds = init_bearer_creds("TOKEN");

	assert_int_equal(creds.type, BEARER);
	assert_string_equal(creds.u.token, "TOKEN");
}

void
test_init_client_creds(void ** state)
{
	struct credentials creds = init_client_creds("ID", "SECRET");

	assert_int_equal(creds.type, CLIENT);
	assert_string_equal(creds.u.client.id, "ID");
	assert_string_equal(creds.u.client.secret, "SECRET");
}

/*******************************************
 *              FIXTURES
 *******************************************/

int
main()
{
	const struct CMUnitTest tests[] = {
		{"initialize w/ bearer token", test_init_bearer_creds},
		{"initialize w/ client id and secret", test_init_client_creds},
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}

