#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <globus_auth.h>
#include "helpers.h"

static void
test_is_uuid(void **state)
{
	assert_int_not_equal(_is_uuid("ed123685-e58d-4ffb-abba-8a33c3fd72ac"), 0);
	assert_int_equal(_is_uuid("https://www.globus.org"), 0);
	assert_int_equal(_is_uuid("support@globus.org"), 0);
}

static void
test_build_list(void **state)
{
	char * list = NULL;

	/* Zero entries. */
	list = _build_list((const char *[]){NULL, NULL}, ',');
	assert_string_equal(list, "");
	test_free(list);

	/* One entry. */
	list = _build_list((const char *[]){"Hello", NULL}, ',');
	assert_string_equal(list, "Hello");
	test_free(list);

	/* Two entries. */
	list = _build_list((const char *[]){"Hello", "There", NULL}, ',');
	assert_string_equal(list, "Hello,There");
	test_free(list);

	/* Thre entries, space delimiter. */
	list = _build_list((const char *[]){"Hello", "There", "World", NULL}, ' ');
	assert_string_equal(list, "Hello There World");
	test_free(list);
}

static void
test_build_kv_list(void **state)
{
	char * list = NULL;

	/* Zero entries */
	list = _build_kv_list(',', NULL);
	assert_string_equal(list, "");
	test_free(list);

	/* One entry */
	list = _build_kv_list(',', "X", "1", NULL);
	assert_string_equal(list, "X=1");
	test_free(list);

	/* Two entries */
	list = _build_kv_list(',', "X", "1", "Y", "2", NULL);
	assert_string_equal(list, "X=1,Y=2");
	test_free(list);
}

static void
test_build_string(void **state)
{
	char * string = NULL;

	/* Zero entries */
	string = _build_string("", "XX");
	assert_string_equal(string, "");
	test_free(string);

	/* One entry */
	string = _build_string("%s", "YY");
	assert_string_equal(string, "YY");
	test_free(string);

	/* Hex integer and char */
	string = _build_string("%x/%c", 32, 'f');
	assert_string_equal(string, "20/f");
	test_free(string);
}

int
main()
{
        const struct CMUnitTest tests[] = {
                cmocka_unit_test(test_is_uuid),
                cmocka_unit_test(test_build_list),
                cmocka_unit_test(test_build_kv_list),
                cmocka_unit_test(test_build_string),
        };
        return cmocka_run_group_tests(tests, NULL, NULL);
}

