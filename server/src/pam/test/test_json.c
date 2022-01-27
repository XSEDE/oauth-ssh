/*
 * System includes.
 */
#include <string.h>
#include <stdlib.h>

/*
 * Local includes.
 */
#include "json.h"
#include "strings.h"
#include "debug.h" // always last

/*******************************************
 *              TESTS
 *******************************************/

void
test_jobj_key_exists(void ** state)
{
	const char * s = "{ \"K1\": \"string\", \"K2\": null}";
	jobj_t * j = jobj_init(s, NULL);
	assert_true(jobj_key_exists(j, "K1"));
	assert_true(jobj_key_exists(j, "K2"));
	assert_false(jobj_key_exists(j, "L"));
	jobj_fini(j);
}

void
test_jobj_get_type(void ** state)
{
	const char * s = "{ \"key\": \"string\"}";
	jobj_t * j = jobj_init(s, NULL);
	assert_int_equal(jobj_get_type(j, "key"), json_type_string);
	jobj_fini(j);

	s = "{ \"key\": 1}";
	j = jobj_init(s, NULL);
	assert_int_equal(jobj_get_type(j, "key"), json_type_int);
	jobj_fini(j);

	s = "{ \"key\": true}";
	j = jobj_init(s, NULL);
	assert_int_equal(jobj_get_type(j, "key"), json_type_boolean);
	jobj_fini(j);

	s = "{ \"key\": {}}";
	j = jobj_init(s, NULL);
	assert_int_equal(jobj_get_type(j, "key"), json_type_object);
	jobj_fini(j);

	s = "{ \"key\": []}";
	j = jobj_init(s, NULL);
	assert_int_equal(jobj_get_type(j, "key"), json_type_array);
	jobj_fini(j);

	s = "{ \"key\": null}";
	j = jobj_init(s, NULL);
	assert_int_equal(jobj_get_type(j, "key"), json_type_null);
	jobj_fini(j);
}

void
test_jobj_get_value(void ** state)
{
	const char * s = "{ \"k1\": {}, \"k2\": []}";
	jobj_t * j = jobj_init(s, NULL);
	assert_non_null(jobj_get_value(j, "k1"));
	assert_non_null(jobj_get_value(j, "k2"));
	jobj_fini(j);
}

void
test_jobj_get_int(void ** state)
{
	const char * s = "{ \"k\": 7 }";
	jobj_t * j = jobj_init(s, NULL);
	assert_int_equal(jobj_get_int(j, "k"), 7);
	jobj_fini(j);
}

void
test_jobj_get_bool(void ** state)
{
	const char * s = "{ \"k1\": true, \"k2\": false }";
	jobj_t * j = jobj_init(s, NULL);
	assert_true(jobj_get_bool(j, "k1"));
	assert_false(jobj_get_bool(j, "k2"));
	jobj_fini(j);
}

void
test_jobj_get_string(void ** state)
{
	const char * s = "{ \"k\": \"hello world\"}";
	jobj_t * j = jobj_init(s, NULL);
	assert_string_equal(jobj_get_string(j, "k"), "hello world");
	jobj_fini(j);
}

void
test_jarr_get_length(void ** state)
{
	const char * s = "{ \"k\": []}";
	jobj_t * jobj = jobj_init(s, NULL);
	jarr_t * jarr = jobj_get_value(jobj, "k");
	assert_int_equal(jarr_get_length(jarr), 0);
	jobj_fini(jobj);

	s = "{ \"k\": [1]}";
	jobj = jobj_init(s, NULL);
	jarr = jobj_get_value(jobj, "k");
	assert_int_equal(jarr_get_length(jarr), 1);
	jobj_fini(jobj);

	s = "{ \"k\": [1,2]}";
	jobj = jobj_init(s, NULL);
	jarr = jobj_get_value(jobj, "k");
	assert_int_equal(jarr_get_length(jarr), 2);
	jobj_fini(jobj);
}

void
test_jarr_get_type(void ** state)
{
	const char * s = "{ \"k\": [\"hello world\"]}";
	jobj_t * jobj = jobj_init(s, NULL);
	jarr_t * jarr = jobj_get_value(jobj, "k");
	assert_int_equal(jarr_get_type(jarr, 0), json_type_string);
	jobj_fini(jobj);
}

void
test_jarr_get_index(void ** state)
{
	const char * s = "{ \"k\": [[], {}]}";
	jobj_t * jobj = jobj_init(s, NULL);
	jarr_t * jarr = jobj_get_value(jobj, "k");
	assert_non_null(jarr_get_index(jarr, 0));
	assert_non_null(jarr_get_index(jarr, 1));
	jobj_fini(jobj);
}

void
test_jarr_get_int(void ** state)
{
	const char * s = "{ \"k\": [1]}";
	jobj_t * jobj = jobj_init(s, NULL);
	jarr_t * jarr = jobj_get_value(jobj, "k");
	assert_int_equal(jarr_get_int(jarr, 0), 1);
	jobj_fini(jobj);
}

void
test_jarr_get_bool(void ** state)
{
	const char * s = "{ \"k\": [true, false]}";
	jobj_t * jobj = jobj_init(s, NULL);
	jarr_t * jarr = jobj_get_value(jobj, "k");
	assert_true(jarr_get_bool(jarr, 0));
	assert_false(jarr_get_bool(jarr, 1));
	jobj_fini(jobj);
}

void
test_jarr_get_string(void ** state)
{
	const char * s = "{ \"k\": [\"hello world\"]}";
	jobj_t * jobj = jobj_init(s, NULL);
	jarr_t * jarr = jobj_get_value(jobj, "k");
	assert_string_equal(jarr_get_string(jarr, 0), "hello world");
	jobj_fini(jobj);
}

/*******************************************
 *              FIXTURES
 *******************************************/

int
main()
{
	const struct CMUnitTest tests[] = {
		{"jobj_key_exists()", test_jobj_key_exists},
		{"jobj_get_type()",   test_jobj_get_type},
		{"jobj_get_value()",  test_jobj_get_value},
		{"jobj_get_int()",    test_jobj_get_int},
		{"jobj_get_bool()",   test_jobj_get_bool},
		{"jobj_get_string()", test_jobj_get_string},
		{"jarr_get_length()", test_jarr_get_length},
		{"jarr_get_type()",   test_jarr_get_type},
		{"jarr_get_index()",  test_jarr_get_index},
		{"jarr_get_int()",    test_jarr_get_int},
		{"jarr_get_bool()",   test_jarr_get_bool},
		{"jarr_get_string()", test_jarr_get_string},
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
