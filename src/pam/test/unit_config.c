/*
 * Copyright The University of Chicago
 * All Rights Reserved.
 */

/*
 * Local includes.
 */
#include "unit_test.h"
#include "../config.c"

/*******************************************
 *              MOCKS
 *******************************************/
#include "mock_stdio.h"

/*******************************************
 *              TESTS
 *******************************************/
#define FPTR  (FILE *)0xDEADBEEF
#define PATH  "path"
#define KEY   "key1"
#define VALUE "value1"

void
test_eperm_on_load(void ** state)
{
	struct fopen_mock m = {.errnum=EPERM, .fptr=NULL};
	expect_string(__wrap_fopen, path, PATH);
	will_return(__wrap_fopen, &m);

	assert_int_equal(config_load(*state, PATH), -m.errnum);
}

void
test_key_with_value(void ** state)
{
	// fopen() the file
	struct fopen_mock m1 = {.errnum=0, .fptr=FPTR};
	expect_string(__wrap_fopen, path, PATH);
	will_return(__wrap_fopen, &m1);

	// fgets() first line
	struct fgets_mock m2 = {.errnum=0, .cptr=KEY " " VALUE};
	will_return(__wrap_fgets, &m2);

	// fgets() eof
	struct fgets_mock m3 = {0, .cptr=NULL};
	will_return(__wrap_fgets, &m3);

	// fclose() file
	struct fclose_mock m4 = {.errnum=0, .num=0};
	expect_value(__wrap_fclose, fp, FPTR);
	will_return(__wrap_fclose, &m4);

	struct config * config = config_init();
	assert_int_equal(config_load(config, PATH), 0);

	char * value;
	config_get_value(config, KEY, &value);
	assert_string_equal(value, VALUE);
	test_free(value);

	config_free(config);
}

void
test_key_without_value(void ** state)
{
	// fopen() the file
	struct fopen_mock m1 = {.errnum=0, .fptr=FPTR};
	expect_string(__wrap_fopen, path, PATH);
	will_return(__wrap_fopen, &m1);

	// fgets() first line
	struct fgets_mock m2 = {.errnum=0, .cptr=KEY};
	will_return(__wrap_fgets, &m2);

	// fgets() eof
	struct fgets_mock m3 = {0, .cptr=NULL};
	will_return(__wrap_fgets, &m3);

	// fclose() file
	struct fclose_mock m4 = {.errnum=0, .num=0};
	expect_value(__wrap_fclose, fp, FPTR);
	will_return(__wrap_fclose, &m4);

	struct config * config = config_init();
	assert_int_equal(config_load(config, PATH), 0);

	char * value;
	config_get_value(config, KEY, &value);
	assert_null(value);

	config_free(config);
}

void
test_comment(void ** state)
{
	// fopen() the file
	struct fopen_mock m1 = {.errnum=0, .fptr=FPTR};
	expect_string(__wrap_fopen, path, PATH);
	will_return(__wrap_fopen, &m1);

	// fgets() first line
	struct fgets_mock m2 = {.errnum=0, .cptr="#"KEY " " VALUE};
	will_return(__wrap_fgets, &m2);

	// fgets() eof
	struct fgets_mock m3 = {0, .cptr=NULL};
	will_return(__wrap_fgets, &m3);

	// fclose() file
	struct fclose_mock m4 = {.errnum=0, .num=0};
	expect_value(__wrap_fclose, fp, FPTR);
	will_return(__wrap_fclose, &m4);

	struct config * config = config_init();
	assert_int_equal(config_load(config, PATH), 0);

	char * value;
	config_get_value(config, KEY, &value);
	assert_null(value);

	config_free(config);
}

/*******************************************
 *              FIXTURES
 *******************************************/

int
main()
{
	const struct CMUnitTest tests[] = {
		{"propogate error on load", test_eperm_on_load},
		{"key w/ value",  test_key_with_value},
		{"key w/o value", test_key_without_value},
		{"comments",      test_comment},
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}

