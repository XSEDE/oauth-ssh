/*
 * Copyright The University of Chicago
 * All Rights Reserved.
 */

/*
 * Local includes.
 */
#include "unit_test.h"
#include "../json.c"

/*******************************************
 *              MOCKS
 *******************************************/

/*******************************************
 *              TESTS
 *******************************************/

static void
test_fail_parse(void ** state)
{
	char * errmsg = NULL;
	assert_null(json_init("{/Xd4[", &errmsg));
	assert_non_null(errmsg);
	free(errmsg);
}

static void
test_get_int(void ** state)
{
	char * errmsg = NULL;
	json_t * json = json_init("{\"key\" : 5}", &errmsg);
	assert_int_equal(json_get_int(json, "key"), 5);
	json_free(json);
}

static void
test_get_bool(void ** state)
{
	char * errmsg = NULL;
	json_t * json = json_init("{\"key\" : true}", &errmsg);
	assert_true(json_get_bool(json, "key"));
	json_free(json);
}

static void
test_get_string(void ** state)
{
	char * errmsg = NULL;
	json_t * json = json_init("{\"key\" : \"value\"}", &errmsg);
	assert_string_equal(json_get_string(json, "key"), "value");
	json_free(json);
}

static void
test_to_int(void ** state)
{
	char * errmsg = NULL;
	json_t * json = json_init("\"5\"", &errmsg);
	assert_int_equal(json_to_int(json), 5);
	json_free(json);
}

static void
test_to_bool(void ** state)
{
	char * errmsg = NULL;
	json_t * json = json_init("\"true\"", &errmsg);
	assert_true(json_to_bool(json));
	json_free(json);
}

static void
test_to_string(void ** state)
{
	char * errmsg = NULL;
	json_t * json = json_init("\"value\"", &errmsg);
	assert_string_equal(json_to_string(json), "value");
	json_free(json);
}

static void
test_access_array(void ** state)
{
	char * errmsg = NULL;
	const char * string = "{\"key\" : [1,true,\"3\"]}";

	json_t * json_object = json_init(string, &errmsg);
	json_t * json_array  = json_get_array(json_object, "key");

	assert_non_null(json_array);
	assert_int_equal(json_array_len(json_array), 3);

	json_t * json_tmp;

	json_tmp = json_array_idx(json_array, 0);
	assert_non_null(json_tmp);
	assert_int_equal(json_to_int(json_tmp), 1);
	json_free(json_tmp);

	json_tmp = json_array_idx(json_array, 1);
	assert_non_null(json_tmp);
	assert_true(json_to_bool(json_tmp));
	json_free(json_tmp);

	json_tmp = json_array_idx(json_array, 2);
	assert_non_null(json_tmp);
	assert_string_equal(json_to_string(json_tmp), "3");
	json_free(json_tmp);

	json_free(json_array);

	json_tmp = json_get_array(json_object, "KeyDoesNotExist");
	assert_null(json_tmp);

	json_free(json_object);
}

static void
test_get_object(void ** state)
{
	char * errmsg = NULL;
	const char * string = "{\"key1\" : {\"key2\" : 1}}";

	json_t * json1 = json_init(string, &errmsg);

	json_t * json2 = json_get_object(json1, "key1");
	assert_int_equal(json_get_int(json2, "key2"), 1);
	json_free(json2);

	json_t * json3 = json_get_object(json1, "key3");
	assert_null(json3);

	json_free(json1);
}

/*******************************************
 *              FIXTURES
 *******************************************/

int
main()
{
	const struct CMUnitTest tests[] = {
		{"propogate error on fail parse", test_fail_parse},
		{"get int value", test_get_int},
		{"get bool value", test_get_bool},
		{"get string value", test_get_string},
		{"convert to int", test_to_int},
		{"convert to bool", test_to_bool},
		{"convert to string", test_to_string},
		{"access array", test_access_array},
		{"get object", test_get_object},
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}
