#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <string.h>
#include <cmocka.h>

#include <globus_auth.h>

static void
test_generate_code_verifier(void **state)
{
	int length = 32;
	char * verifier = generate_code_verifier(length);
	assert_int_equal(strlen(verifier), length);
	test_free(verifier);
}

int
main()
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_generate_code_verifier),
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}
