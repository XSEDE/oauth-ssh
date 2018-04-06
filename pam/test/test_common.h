/*
 * Copyright The University of Chicago
 * All Rights Reserved.
 */
#ifndef TEST_COMMON_H
#define TEST_COMMON_H

#define ASSERT(x) if (__expect(#x, (x), __test_counter++) != TEST_SUCCESS) return TEST_FAILURE

extern int __test_counter;

int
__expect(const char * desc, int test, int counter);

enum {
	TEST_SUCCESS,
	TEST_FAILURE,
};

#endif /* TEST_COMMON_H */
