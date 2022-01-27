/*
 * System includes.
 */
#include <string.h>
#include <stdlib.h>

#define PAM_SM_AUTH
#include <security/pam_modules.h>

/*
 * Local includes.
 */
#include "base64.h"
#include "debug.h" // always last

/*******************************************
 *              TESTS
 *******************************************/
void
test_base64_encode(void ** state)
{
	const char * input = "ABCDEFG1234567";
	char * output = base64_encode(input);
	assert_string_equal(output, "QUJDREVGRzEyMzQ1Njc=");
	free(output);
}

void
test_base64_decode(void ** state)
{
	const char * input = "QUJDREVGRzEyMzQ1Njc=";
	char * output = base64_decode(input);
	assert_string_equal(output, "ABCDEFG1234567");
	free(output);
}

/*******************************************
 *              FIXTURES
 *******************************************/

int
main()
{
	const struct CMUnitTest tests[] = {
		{"base64 encode", test_base64_encode},
		{"base64 decode", test_base64_decode},
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}

