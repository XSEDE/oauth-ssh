/*
 * Copyright The University of Chicago
 * All Rights Reserved.
 */

/*
 * System includes.
 */
#include <string.h>
#include <stdlib.h>

/*
 * Local includes.
 */
#include "unit_test.h"
#include "../http.c"

/*******************************************
 *              MOCKS
 *******************************************/

/*******************************************
 *              TESTS
 *******************************************/

const int small_size = 128;
const int large_size = 1024 * 1024;

static char *
random_text(int size)
{
	char charset[] = "0123456789"
	                 "abcdefghijklmnopqrstuvwxyz"
	                 "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

	char * s = calloc(size + 1, sizeof(char));
	for (int i = 0; i < size; i++)
	{
		s[i] = charset[random()%(sizeof(charset)-1)];
	}
	return s;
}


typedef size_t (*callback)(char * ptr,
                           size_t size, 
                           size_t nmemb, 
                           void * userdata);

int
test(char * buffer, int size, callback cb, void * arg)
{
	const int max_cb_size = 16*1024;

	int offset = 0;

	while (offset < size)
	{
		int length = size - offset;
		if (length > max_cb_size)
			length = max_cb_size;

		int retval = cb(&buffer[offset], length, 1, arg);
		assert_true(retval > 0);
		offset += retval;
	}

	return offset;
}

void
test_small_response(void ** state)
{
	struct response_body response = {0, NULL};

	char * msg = random_text(small_size);
	int length = test(msg, small_size, capture_response, &response);

	assert_int_equal(small_size, length);
	assert_int_equal(small_size, strlen(response.body));
	assert_string_equal(response.body, msg);
	free(response.body);
	free(msg);
}

void
test_large_response(void ** state)
{
	struct response_body response = {0, NULL};

	char * msg = random_text(large_size);
	int length = test(msg, large_size, capture_response, &response);

	assert_int_equal(large_size, length);
	assert_int_equal(large_size, strlen(response.body));
	assert_string_equal(response.body, msg);
	free(response.body);
	free(msg);
}

/*******************************************
 *              FIXTURES
 *******************************************/

int
main()
{
	const struct CMUnitTest tests[] = {
		{"small response", test_small_response},
		{"large response", test_large_response},
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}
