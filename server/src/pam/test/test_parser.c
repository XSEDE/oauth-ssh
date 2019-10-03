/*
 * System includes.
 */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/*
 * Local includes.
 */
#include "parser.h"
#include "debug.h" // always last

#define EOL "\n"
#undef EOF
#define EOF NULL

/*******************************************
 *              MOCKS
 *******************************************/
char *
fgets(char *s, int size, FILE *stream)
{
	char * buffer = mock_ptr_type(char *);
	if (!buffer)
	{
		s[0] = '\0';
		return NULL;
	}

	strcpy(s, buffer);
	return s;
}

/*******************************************
 *              TESTS
 *******************************************/
void
test_empty_file(void ** state)
{
	will_return(fgets, EOF);
	char * key;
	char ** values;
	assert_false(read_next_pair(NULL, &key, &values));
	assert_null(key);
	assert_null(values);
}

void
test_comment(void ** state)
{
	will_return(fgets, "# This is a comment" EOL);
	will_return(fgets, EOF);

	char * key;
	char ** values;
	assert_false(read_next_pair(NULL, &key, &values));
	assert_null(key);
	assert_null(values);
}

void
test_one_line(void ** state)
{
	char * key;
	char ** values;
	will_return(fgets, "key value" EOL);

	assert_true(read_next_pair(NULL, &key, &values));

	assert_string_equal(key, "key");
	assert_string_equal(values[0], "value");
	assert_null(values[1]);

	free(key);
	free(values[0]);
	free(values);
}

void
test_key_no_value(void ** state)
{
	char * key;
	char ** values;
	will_return(fgets, "key " EOL);

	assert_true(read_next_pair(NULL, &key, &values));

	assert_string_equal(key, "key");
	assert_null(values);

	free(key);
}

void
test_key_with_comment(void ** state)
{
	char * key;
	char ** values;
	will_return(fgets, "key #value" EOL);

	assert_true(read_next_pair(NULL, &key, &values));

	assert_string_equal(key, "key");
	assert_null(values);

	free(key);
}

void
test_key_value_with_comment(void ** state)
{
	char * key;
	char ** values;
	will_return(fgets, "key value #comment" EOL);

	assert_true(read_next_pair(NULL, &key, &values));

	assert_string_equal(key, "key");
	assert_string_equal(values[0], "value");
	assert_null(values[1]);

	free(key);
	free(values[0]);
	free(values);
}

//void
//test_long_line(void ** state)
//{
//	char * key;
//	char ** values;
//
//	char line1[PARSER_READ_LEN];
//	char line2[PARSER_READ_LEN];
//
//	snprintf(line1, sizeof(line1), "%-*s",  PARSER_READ_LEN-1, "key");
//	snprintf(line2, sizeof(line2), "%*s\n", PARSER_READ_LEN-2, "value");
//
//	will_return(fgets, line1);
//	will_return(fgets, line2);
//
//	assert_true(read_next_pair(NULL, &key, &values));
//
//	assert_string_equal(key, "key");
//	assert_string_equal(values[0], "value");
//	assert_null(values[1]);
//
//	free(key);
//	free(values[0]);
//	free(values);
//}

void
test_space_delim_values(void ** state)
{
	char * key;
	char ** values;
	will_return(fgets, "key value1  value2" EOL);

	assert_true(read_next_pair(NULL, &key, &values));

	assert_string_equal(key, "key");
	assert_string_equal(values[0], "value1");
	assert_string_equal(values[1], "value2");
	assert_null(values[2]);

	free(key);
	free(values[0]);
	free(values[1]);
	free(values);
}

void
test_comma_delim_values(void ** state)
{
	char * key;
	char ** values;
	will_return(fgets, "key value1,value2" EOL);

	assert_true(read_next_pair(NULL, &key, &values));

	assert_string_equal(key, "key");
	assert_string_equal(values[0], "value1");
	assert_string_equal(values[1], "value2");
	assert_null(values[2]);

	free(key);
	free(values[0]);
	free(values[1]);
	free(values);
}

int
main()
{
	const struct CMUnitTest tests[] = {
		{"empty file", test_empty_file},
		{"comment", test_comment},
		{"one line", test_one_line},
		{"key w/o values", test_key_no_value},
		{"key w/ comment", test_key_with_comment},
		{"key, value w/ comment", test_key_value_with_comment},
//		{"one long line", test_long_line},
		{"space delimiter", test_space_delim_values},
		{"comma delimiter", test_comma_delim_values},
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
