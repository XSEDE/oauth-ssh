/*
 * Copyright The University of Chicago
 * All Rights Reserved.
 */

#include <stdlib.h>

#include "../acct_map.h"
#include "../utilities.h"
#include "unit_test.h"


static void
test_get_provider(void ** state)
{
	const char * get_provider(const char * id_username);

	assert_string_equal(get_provider("user@provider"), "provider");
	assert_null(get_provider("abcdef"));
}

static void
test_get_username(void ** state)
{
	int get_username(const char * id_username);

	assert_int_equal(get_username("user@provider"), 4);
	assert_int_equal(get_username("abcdef"), 0);
}

static void
test_identity_is_from_provider(void ** state)
{
	int identity_is_from_provider(const char * id_username,
	                              const char * idp_suffix);

	assert_true(identity_is_from_provider("user@provider", "provider") != 0);
	assert_false(identity_is_from_provider("user@aprovider","provider"));
	assert_false(identity_is_from_provider("abcdefg","provider"));
}

#include <stdio.h>
static void
test_acct_map_idp_suffix(void ** state)
{
	char ** accts = NULL;

	struct auth_identity id1 = {NULL, "user@example.com", 0, NULL, NULL, NULL};

	struct auth_identity * identities[] = { &id1, NULL, };

	accts = acct_map_idp_suffix("example.com", identities);

	assert_true(string_in_list("user", accts));

	acct_map_free_list(accts);
}

int
main()
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_get_provider),
		cmocka_unit_test(test_get_username),
		cmocka_unit_test(test_identity_is_from_provider),
		cmocka_unit_test(test_acct_map_idp_suffix),
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}

