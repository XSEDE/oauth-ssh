/*
 * Copyright The University of Chicago
 * All Rights Reserved.
 */


/*
 * Local includes.
 */
#include "unit_test.h"
#include "../strings.c"

static void
test_join(void ** state)
{
	const char * const list1[] = {"bob", NULL};
	char * s1 = join(list1, '+');
	assert_string_equal(s1, "bob");
	free(s1);

	const char * const list2[] = {"foo", "bar", NULL};
	char * s2 = join(list2, '&');
	assert_string_equal(s2, "foo&bar");
	free(s2);

	const char * const list3[] = {"foo", "bar", "zoo", NULL};
	char * s3 = join(list3, 0);
	assert_string_equal(s3, "foobarzoo");
	free(s3);
}

static void
test_concat(void ** state)
{
	char * s = concat("prefix", "suffix");
	assert_int_equal(strcmp(s, "prefixsuffix"), 0);
	free(s);
}

static void
test_append(void ** state)
{
	char * s_in  = strdup("prefix");
	char * s_out = append(&s_in, "suffix");
	assert_int_equal(s_in, s_out);
	assert_int_equal(strcmp(s_in, "prefixsuffix"), 0);
	free(s_in);
}

static void
test_split(void ** state)
{
	char ** strings = split("ABC", ' ');
	assert_string_equal(strings[0], "ABC");
	assert_null(strings[1]);
	free(strings[0]);
	free(strings);

	strings = split("A B C", ' ');
	assert_string_equal(strings[0], "A");
	assert_string_equal(strings[1], "B");
	assert_string_equal(strings[2], "C");
	assert_null(strings[3]);
	free(strings[0]);
	free(strings[1]);
	free(strings[2]);
	free(strings);
}

static void
test_string_in_list(void ** state)
{
	const char * list[4] = {"foo", "bob", "bar", NULL};

	assert_true(string_in_list("bob", list));
	assert_false(string_in_list("ted", list));
	assert_false(string_in_list("bob", NULL));
}

static void
test_safe_strdup(void ** state)
{
	assert_null(safe_strdup(NULL));

	char * contents = "Hello World";

	char * s = safe_strdup(contents);
	assert_string_equal(s, contents);
	free(s);
}

int
main()
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_join),
		cmocka_unit_test(test_concat),
		cmocka_unit_test(test_append),
		cmocka_unit_test(test_split),
		cmocka_unit_test(test_string_in_list),
		cmocka_unit_test(test_safe_strdup),
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}
