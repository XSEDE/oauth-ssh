/*
 * Copyright The University of Chicago
 * All Rights Reserved.
 */

#include <stdio.h>
#include "test_common.h"

int __test_counter = 1;

int
__expect(const char * desc, int test, int counter)
{
	printf("%d ", counter);
	if (!test)
		printf("not ");
	printf("ok - %s\n", desc);

	if (!test)
		return TEST_FAILURE;

	return TEST_SUCCESS;
}

