/*
 * Copyright The University of Chicago
 * All Rights Reserved.
 */

#include <stdlib.h>
#include <string.h>
#define PAM_SM_AUTH
#include <security/pam_modules.h>

#ifdef UNIT_TEST
  #include "test/unit_test.h"
#endif

#include "utilities.h"
#include "constants.h"

/*
 * Send a single message, echo disabled, display the given prompt,
 * return a copy of the access token.
 */
int
get_client_access_token(struct pam_conv *  conv,
                        const char      *  prompt,
                        char            ** access_token)
{
	int pam_err = PAM_SUCCESS;
	struct pam_message msg;
	const struct pam_message * msgp;
	struct pam_response      * resp;

	*access_token = NULL;

	/*
	 * We need to ask for the user's password. If this is SSH with password
	 * authentication enabled, the user has already supplied the password and
	 * it will be copied in as the response to any message we send with
	 * 'echo off'. If this is SSH with challenge response enabled, the user
	 * will see our prompt.
	 */
	msg.msg_style = PAM_PROMPT_ECHO_OFF;
	msg.msg       = prompt;
	msgp          = &msg;
	resp          = NULL;
	pam_err       = (*conv->conv)(1, &msgp, &resp, conv->appdata_ptr);

	if (pam_err == PAM_SUCCESS)
	{
		*access_token = resp->resp;
		free(resp);
	}

	return pam_err;
}

/*
 * Send a single message, type text info, display the given prompt.
 */
int
display_client_message(struct pam_conv * conv,
                       const char      * message)
{
	int                pam_err = PAM_SUCCESS;
	struct pam_message msg;
	const struct pam_message * msgp;
	struct pam_response      * resp;

	msg.msg_style = PAM_TEXT_INFO;
	msg.msg       = message;
	msgp          = &msg;

	resp      = NULL;
	pam_err   = (*conv->conv)(1, &msgp, &resp, conv->appdata_ptr);
	if (resp != NULL && pam_err == PAM_SUCCESS)
	{
		free(resp->resp);
		free(resp);
	}

	return pam_err;
}

char * 
join_string_list(const char * const strings[],
                 const char delimiter)
{
	if (!strings || !strings[0])
		return NULL;

	int length = 0;
	for (int i = 0; strings[i]; i++)
	{
		length += strlen(strings[i]) + 1;
	}

	char * joined_string = calloc(length, sizeof(char));

	int offset = 0;
	for (int i = 0; strings[i]; i++)
	{
		if (i != 0)
			joined_string[offset++] = delimiter;
		strcpy(&joined_string[offset], strings[i]);
		offset += strlen(strings[i]);
	}

	return joined_string;
}

char *
join_strings(const char * string1,
             const char * string2,
             const char   delimiter)
{
	const char * list[] = {string1, string2, NULL};
	return join_string_list(list, delimiter);
}

char *
build_acct_map_msg(const char * const accounts[])
{
	char * account_list = join_string_list(accounts, ',');
	if (!account_list)
	{
		char * acct_map_msg = malloc(strlen(NoLocalAcctMsg)+1);
		strcpy(acct_map_msg, NoLocalAcctMsg);
		return acct_map_msg;
	}

	char * acct_map_msg = join_strings(LocalAcctMsgPrefix, account_list, ' ');
	free (account_list);
	return acct_map_msg;
}

int
string_in_list(const char * string,
               const char * const list[])
{
	for (int i = 0; list && list[i]; i++)
	{
		if (strcmp(string, list[i]) == 0)
			return 1;
	}

	return 0;
}
