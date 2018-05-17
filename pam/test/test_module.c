/*
 * Copyright The University of Chicago
 * All Rights Reserved.
 */

#include <dlfcn.h>
#include <stdio.h>

#include "unit_test.h"

static void
test_load_module(void **state)
{
	void * handle = dlopen("../.libs/pam_globus.so.0.0.0", RTLD_NOW);

	if (!handle)
		printf("%s\n", dlerror());
	assert_non_null(handle);

	assert_non_null(dlsym(handle, "pam_sm_authenticate"));
	assert_non_null(dlsym(handle, "pam_sm_setcred"));

	assert_int_equal(dlclose(handle), 0);
}

int
main()
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_load_module),
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}
