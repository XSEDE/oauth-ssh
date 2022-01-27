/*
 * System includes.
 */
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#define PAM_SM_AUTH
#include <security/pam_modules.h>

/*
 * Local includes.
 */
#include "strings.h"
#include "debug.h" // always last

/*******************************************
 *              TESTS
 *******************************************/
void
test_join_0_strings(void ** state)
{
	char * ptr = join((const char *[]){NULL}, ',');
	assert_null(ptr);
}

void
test_join_1_string(void ** state)
{
	char * str = "A";
	char * ptr = join((const char *[]){str, NULL}, ',');
	assert_string_equal(ptr, str);
	free(ptr);
}

void
test_join_2_strings(void ** state)
{
#define STR1 "A"
#define STR2 "B"
#define DELIM ","

	char * ptr = join((const char *[]){STR1, STR2, NULL}, *DELIM);
	assert_string_equal(ptr, STR1 DELIM STR2);
	free(ptr);
}

void
test_join_no_delim(void ** state)
{
	char * ptr = join((const char *[]){STR1, STR2, NULL}, 0);
	assert_string_equal(ptr, STR1 STR2);
	free(ptr);
}

void
test_append_strings(void ** state)
{
	char * str = strdup(STR1);
	append(&str, STR2);
	assert_string_equal(str, STR1 STR2);
	free(str);
}

void
test_insert_into_array(void ** state)
{
	char ** array = calloc(2, sizeof(char *));
	array[0] = strdup(STR1);
	insert(&array, STR2);
	assert_string_equal(array[0], STR1);
	assert_string_equal(array[1], STR2);
	assert_null(array[2]);
	free_array(array);
}

void
test_insert_null_array(void ** state)
{
	char ** array = NULL;
	insert(&array, STR1);
	assert_string_equal(array[0], STR1);
	assert_null(array[1]);
	free_array(array);
}

void
test_sformat(void ** state)
{
	errno = 1;
	char * ptr = sformat("s:%s d:%d m:%m", "STR", 7);
	assert_string_equal(ptr, "s:STR d:7 m:Operation not permitted");
	free(ptr);
}

void
test_key_in_list(void ** state)
{
	assert_true(key_in_list((const char*[]){STR1, STR2, NULL}, STR1));
}

void
test_key_not_in_list(void ** state)
{
	assert_false(key_in_list((const char*[]){STR2, NULL}, STR1));
}

void
test_suffix_match(void ** state)
{
	assert_true(string_has_suffix("johndoe@example.com", "example.com"));
}

void
test_suffix_mismatch(void ** state)
{
	assert_false(string_has_suffix("johndoe@example.com", "abc.com"));
}

/*******************************************
 *              FIXTURES
 *******************************************/

int
main()
{
	const struct CMUnitTest tests[] = {
		{"join 0 strings", test_join_0_strings},
		{"join 1 string", test_join_1_string},
		{"join 2 strings", test_join_2_strings},
		{"join without delimter", test_join_no_delim},
		{"append 2 strings", test_append_strings},
		{"insert into array", test_insert_into_array},
		{"insert into null array", test_insert_null_array},
		{"sformat", test_sformat},
		{"key in list", test_key_in_list},
		{"key not in list", test_key_not_in_list},
		{"suffix match",    test_suffix_match},
		{"suffix mismatch", test_suffix_mismatch},
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}
