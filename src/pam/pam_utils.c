/*
 * Copyright The University of Chicago
 * All Rights Reserved.
 */

/*
 * This file used for functions used by pam.c that require unit testing.
 * This avoids to the need to mock dependencies any more than necessary.
 */

int
get_array_len(void ** array)
{
	int count;
	for (count = 0; array && array[count]; count++);
	return count;
}

