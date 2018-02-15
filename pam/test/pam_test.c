/*
 * Copyright The University of Chicago
 * All Rights Reserved.
 */

#define PAM_SM_AUTH
#include <security/pam_modules.h>
#include <security/pam_appl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "test_common.h"

struct pam_conv_expect {
	char * resp;
	int return_value;
} pam_conv_expect;

static int
_pam_conv(int num_msg, const struct pam_message **msg, struct pam_response **resp, void *appdata_ptr)
{
	*resp = calloc(num_msg, sizeof(struct pam_response));
	(*resp)[0].resp = pam_conv_expect.resp;
	return pam_conv_expect.return_value;
}

struct pam_get_item_expect {
	union {
		 struct pam_conv conv;
	} u;

	int return_value;
} static pam_get_item_expect;

int 
__wrap_pam_get_item(const pam_handle_t *pamh, int item_type, const void **item)
{
	switch (item_type)
	{
	case PAM_CONV:
		*((struct pam_conv **)item) = &pam_get_item_expect.u.conv;
		break;
	default:
		break;
	}

	return pam_get_item_expect.return_value;
}

int
_get_client_access_token_test()
{
	extern int _get_client_access_token(pam_handle_t * pamh, const char ** access_token);
	const char * access_token = NULL;

	/*
	 * Success.
	 */
	pam_get_item_expect = (struct pam_get_item_expect) {{_pam_conv, (void **)0xABCD1234}, PAM_SUCCESS};
	pam_conv_expect = (struct pam_conv_expect) {"fee fi fo fum", PAM_SUCCESS};

	ASSERT(_get_client_access_token(NULL, &access_token) == PAM_SUCCESS);
	ASSERT(strcmp(access_token, pam_conv_expect.resp) == 0);

	/*
	 * Failure 1.
	 */
	pam_get_item_expect = (struct pam_get_item_expect) {{_pam_conv, (void **)0xABCD1234}, PAM_SYSTEM_ERR};
	pam_conv_expect = (struct pam_conv_expect) {"fee fi fo fum", PAM_SUCCESS};

	ASSERT(_get_client_access_token(NULL, &access_token) != PAM_SUCCESS);

	/*
	 * Failure 2.
	 */
	pam_get_item_expect = (struct pam_get_item_expect) {{_pam_conv, (void **)0xABCD1234}, PAM_SUCCESS};
	pam_conv_expect = (struct pam_conv_expect) {"fee fi fo fum", PAM_CONV_ERR};

	ASSERT(_get_client_access_token(NULL, &access_token) != PAM_SUCCESS);

	return 0;
}

int
_build_account_msg_test()
{
	extern char * _build_account_msg(const char * const * account_list);

	ASSERT(strcmp(_build_account_msg((const char * const []){"bob", NULL}), "You can log in as bob") == 0);
	ASSERT(strcmp(_build_account_msg((const char * const []){"foo", "bar", NULL}), "You can log in as foo, bar") == 0);
	ASSERT(strcmp(_build_account_msg((const char * const []){NULL}), "You have no local accounts") == 0);
	return 0;
}

#ifdef NOT
int
_display_valid_accounts_test()
{
	extern int _display_valid_accounts(pam_handle_t * pamh, const char * const * account_list);
	const char * const * account_list;

	/*
	 * Success.
	 */
	pam_get_item_expect = (struct pam_get_item_expect) {{_pam_conv, (void **)0xABCD1234}, PAM_SUCCESS};
	pam_conv_expect = (struct pam_conv_expect) {"fee fi fo fum", PAM_SUCCESS};

	ASSERT(_get_client_access_token(NULL, &access_token) == PAM_SUCCESS);
	ASSERT(strcmp(access_token, pam_conv_expect.resp) == 0);

	/*
	 * Failure 1.
	 */
	pam_get_item_expect = (struct pam_get_item_expect) {{_pam_conv, (void **)0xABCD1234}, PAM_SYSTEM_ERR};
	pam_conv_expect = (struct pam_conv_expect) {"fee fi fo fum", PAM_SUCCESS};

	ASSERT(_get_client_access_token(NULL, &access_token) != PAM_SUCCESS);

	/*
	 * Failure 2.
	 */
	pam_get_item_expect = (struct pam_get_item_expect) {{_pam_conv, (void **)0xABCD1234}, PAM_SUCCESS};
	pam_conv_expect = (struct pam_conv_expect) {"fee fi fo fum", PAM_CONV_ERR};

	ASSERT(_get_client_access_token(NULL, &access_token) != PAM_SUCCESS);

	return 0;
}
#endif

int
_is_string_in_list_test()
{
	extern int _is_string_in_list(const char * string, const char * const * list);

	ASSERT(_is_string_in_list("bob", (const char * const []) {"foo", "bob", "bar", NULL}) != 0);
	ASSERT(_is_string_in_list("bob", (const char * const []) {"foo", "bar", NULL}) == 0);
	ASSERT(_is_string_in_list("bob", (const char * const []) {NULL}) == 0);
}

int
main()
{
	return _get_client_access_token_test() || 
	       _build_account_msg_test()       ||
	       _is_string_in_list_test();
}
