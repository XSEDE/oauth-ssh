/*
 * Copyright The University of Chicago
 * All Rights Reserved.
 */

/*
 * Local includes.
 */
#include "unit_test.h"
#include "../scope.c"

#define SCOPE_FOUND(suffix, scope) \
{ \
	char * errmsg = NULL; \
	int found = has_scope(suffix, (char * []){scope, NULL}, &errmsg); \
	assert_int_equal(found, 1); \
	assert_null(errmsg); \
}

#define SCOPE_NOT_FOUND(suffix, scope) \
{ \
	char * errmsg = NULL; \
	int found = has_scope(suffix, (char * []){scope, NULL}, &errmsg); \
	assert_int_equal(found, 0); \
	assert_null(errmsg); \
}

/*******************************************
 *              MOCKS
 *******************************************/

/*******************************************
 *              TESTS
 *******************************************/

static void
test_has_scope(void ** state)
{
	// Valid FQDN Midfix
	SCOPE_FOUND("ssh", SCOPE_PREFIX "ssh.globus.org/ssh");
	SCOPE_FOUND("ssh", SCOPE_PREFIX "s-h.globus.org/ssh");
	SCOPE_FOUND("ssh", SCOPE_PREFIX "1s.globus.org/ssh");
	SCOPE_FOUND("ssh", SCOPE_PREFIX "s1.globus.org/ssh");
	SCOPE_FOUND("ssh", SCOPE_PREFIX "s.globus.org/ssh");

	// Invalid Prefix
	SCOPE_NOT_FOUND("ssh", "https://www.globus.org/scopes/ssh.globus.org/ssh");

	// Invalid Midfix DNS Label
	SCOPE_NOT_FOUND("ssh", SCOPE_PREFIX "ss-.globus.org/ssh");
	SCOPE_NOT_FOUND("ssh", SCOPE_PREFIX "-sh.globus.org/ssh");
	SCOPE_NOT_FOUND("ssh", SCOPE_PREFIX "1.globus.org/ssh");

	// Invalid Midfix FQDN
	SCOPE_NOT_FOUND("ssh", SCOPE_PREFIX "globus.org/ssh");

	// Invalid Suffix
	SCOPE_NOT_FOUND("ssh", SCOPE_PREFIX "globus.org/ssh/ssh");
}

int
main()
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_has_scope),
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}

