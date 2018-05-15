/*
 * Copyright The University of Chicago
 * All Rights Reserved.
 */

#include <string.h>
#define PAM_SM_AUTH
#include <security/pam_modules.h>

#include "unit_test.h"

#include "../utilities.h"
#include "../constants.h"

struct test_conv_data {
	int                   num_msg;
	struct pam_message *  msg;
	const char         ** response;
	int                   return_value;
};

int
test_conv(int                         num_msg, 
          const struct pam_message ** msg,
          struct pam_response      ** resp, 
          void                     *  appdata_ptr)
{
	struct test_conv_data * data = appdata_ptr;

	assert_int_equal(num_msg, data->num_msg);

	*resp = test_calloc(num_msg, sizeof(struct pam_response));
	for (int i = 0; i < num_msg; i++)
	{
		assert_int_equal(msg[i]->msg_style, data->msg[i].msg_style);
		assert_string_equal(msg[i]->msg, data->msg[i].msg);

		(*resp)[i].resp = test_malloc(strlen(data->response[i])+1);
		strcpy((*resp)[i].resp, data->response[i]);
	}

	return data->return_value;
}

static void
test_get_access_token(void **state)
{
	struct pam_message msg[] = {{PAM_PROMPT_ECHO_OFF, "Hello World"}};
	const char * responses[] = {"Goodbye World"};

	struct test_conv_data data = {1, 
	                              msg,
	                              responses,
	                              PAM_SUCCESS};

	struct pam_conv conv = (struct pam_conv){test_conv, &data};

	char * access_token = NULL;
	int ret = get_client_access_token(&conv,
	                                  data.msg[0].msg,
	                                  &access_token);

	assert_int_equal(ret, data.return_value);
	assert_string_equal(access_token, data.response[0]);
	test_free(access_token);
}

static void
test_display_client_message(void **state)
{
	struct pam_message msg[] = {{PAM_TEXT_INFO, "Hello World"}};
	const char * responses[] = {""};

	struct test_conv_data data = {1, 
	                              msg,
	                              responses,
	                              PAM_SUCCESS};

	struct pam_conv conv = (struct pam_conv){test_conv, &data};

	int ret = display_client_message(&conv, data.msg[0].msg);

	assert_int_equal(ret, data.return_value);
}

static void
test_join_string_list(void ** state)
{
	assert_null(join_string_list(NULL, '+'));
	assert_null(join_string_list((const char * []){NULL}, '+'));

	const char * list[3] = {"bob", NULL};
	assert_string_equal(join_string_list(list, '+'), "bob");

	list[0] = "foo";
	list[1] = "bar";
	list[2] = NULL;
	assert_string_equal(join_string_list(list, '&'), "foo&bar");
}

static void
test_join_strings(void ** state)
{
	assert_null(join_strings(NULL, NULL, '+'));

	char * str = join_strings("bob", NULL, '+');
	assert_string_equal(str, "bob");
	test_free(str);

	str = join_strings("foo", "bar", '+');
	assert_string_equal(str, "foo+bar");
	test_free(str);
}

static void
test_build_acct_map_msg(void ** state)
{
	char * msg = build_acct_map_msg(NULL);
	assert_string_equal(msg, NoLocalAcctMsg);
	test_free(msg);

	const char * acct_list[] = {NULL, NULL, NULL};
	msg = build_acct_map_msg(acct_list);
	assert_string_equal(msg, NoLocalAcctMsg);
	test_free(msg);

	acct_list[0] = "foo";
	msg = build_acct_map_msg(acct_list);
	assert_true(strstr(msg, LocalAcctMsgPrefix) != NULL);
	assert_true(strstr(msg, "foo") != NULL);
	test_free(msg);

	acct_list[1] = "bar";
	msg = build_acct_map_msg(acct_list);
	assert_true(strstr(msg, LocalAcctMsgPrefix) != NULL);
	assert_true(strstr(msg, "foo,bar") != NULL);
	test_free(msg);
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
test_has_scope(void ** state)
{
	const char * list[] = {
		"https://auth.globus.org/scopes/ssh.demo.globus.org/ssh",
		NULL,
	};

	assert_true(has_scope(list[0], list));
	assert_false(has_scope("openid", list));
}

int
main()
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_get_access_token),
		cmocka_unit_test(test_display_client_message),
		cmocka_unit_test(test_join_strings),
		cmocka_unit_test(test_build_acct_map_msg),
		cmocka_unit_test(test_build_acct_map_msg),
		cmocka_unit_test(test_string_in_list),
		cmocka_unit_test(test_has_scope),
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}
