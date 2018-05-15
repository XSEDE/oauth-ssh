#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

/*
 * Simple test case to make sure we have no missing symbols
 * in the library.
 */
static void
test_missing_symbols(void **state) {
    (void) state; /* unused */
}

int
main()
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_missing_symbols),
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}
