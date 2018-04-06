/*
 * Copyright The University of Chicago
 * All Rights Reserved.
 */

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <syslog.h>

#define PAM_SM_AUTH
#include <security/pam_modules.h>

#include <account_mapping.h>
#include <globus_auth.h>

#include "credentials.h"

#ifdef UNIT_TEST
 #define static
#endif /* UNIT_TEST */

// XXX
extern const char * SSHServiceID;
extern const char * SSHServiceSecret;

/*
 * Notes about the "SSH for Globus Auth" PAM module design
 *
 * This PAM module receives the client's access token via the password
 * prompt. There are two ways to do this with SSH, each with caveats. These
 * options are controlled within sshd_config. Either option requires "UsePAM yes".
 *
 * PasswordAuthentication Yes
 *    When enabled, the user will be prompted for the access token *before*
 *    the PAM module is called. SSH will enforce internal, valid account 
 *    checks before calling PAM. When we use the conversation function with
 *    PAM_PROMPT_ECHO_OFF, the password is given to us. This presents the 
 *    following issues:
 *       A) We do not get an opportunity to change the password prompt language
 *       B) Any response we send with PAM_TEXT_INFO (aka mapped accounts), is
 *          queued and display only after *successful* login
 *
 * ChallengeResponseAuthentication Yes
 *    When enabled, the SSH service allows the PAM module to perform the
 *    entirety of the conversation. We can change the password prompt and 
 *    our PAM_TEXT_INFO messages are displayed inline. However, there are
 *    a couple issues:
 *       A) SSHD will not perform account validation
 *       B) SSHD will not perform zero-length password checks
 *       C) SSHD will not perform root-login checks
 *       D) It is possible that some sites may already use this option for
 *          another authentication mechanism (Two Factor)
 */

/*
 * This PAM module must be 'required' or 'requisite' (unverified). 'sufficient' causes
 * globus-mapping to retry.
 */

static int
_get_client_access_token(pam_handle_t * pamh, const char ** access_token)
{
	int pam_err = PAM_SUCCESS;
	struct pam_conv * conv = NULL;
	struct pam_message msg;
	const struct pam_message * msgp;
	struct pam_response      * resp;

	*access_token = NULL;

	pam_err = pam_get_item(pamh, PAM_CONV, (const void **)&conv);
	if (pam_err != PAM_SUCCESS)
		return pam_err;

	/*
	 * We need to ask for the user's password. If this is SSH with password
	 * authentication enabled, the user has already supplied the password and
	 * it will be copied in as the response to any message we send with 'echo off'.
	 * If this is SSH with challenge response enabled, the user will see our prompt.
	 */
	msg.msg_style = PAM_PROMPT_ECHO_OFF;
	msg.msg       = "Enter your Globus Auth token: ";
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

static int
_display_client_message(pam_handle_t * pamh, const char * message)
{
	int                pam_err = PAM_SUCCESS;
	struct pam_conv  * conv    = NULL;
	struct pam_message msg;
	const struct pam_message * msgp;
	struct pam_response      * resp;

	pam_err = pam_get_item(pamh, PAM_CONV, (const void **)&conv);
	if (pam_err != PAM_SUCCESS)
		return pam_err;

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

static char *
_build_account_msg(const char * const * account_list)
{
	const char * msg_prefix = "You can log in as ";

	if (account_list == NULL || account_list[0] == NULL)
		return strdup("You have no local accounts");

	int length = strlen(msg_prefix);
	for (int i = 0; account_list[i]; i++)
	{
		length += strlen(account_list[i]) + 2; /* ', ' */
	}

	char * msg = calloc(length+1, sizeof(char));
	strcpy(msg, msg_prefix);

	for (int i = 0; account_list[i]; i++)
	{
		strcat(msg, account_list[i]);
		if (account_list[i+1])
			strcat(msg, ", ");
	}

	return msg;
}

static int
_display_valid_accounts(pam_handle_t * pamh, const char * const * account_list)
{
	char * message = _build_account_msg(account_list);
	int pam_err = _display_client_message(pamh, message);
	free(message);
	return pam_err;
}

static int
_get_requested_account(pam_handle_t * pamh, const char ** local_account)
{
	int pam_err = PAM_SUCCESS;

	*local_account = NULL;

	return pam_get_user(pamh, local_account, NULL);
}

static int
_is_string_in_list(const char * string, const char * const * list)
{
	for (int i = 0; list && list[i]; i++)
	{
		if (strcmp(string, list[i]) == 0)
			return 1;
	}

	return 0;
}

// XXX Generalize this
static const char * SSHScope = "https://auth.globus.org/scopes/ssh.sandbox.globuscs.info/ssh";

// XXX Handle this when we have multiple FQDNs
static int
_has_scope(const char * Scope, const char * const * ListOfScopes)
{
	return _is_string_in_list(Scope, ListOfScopes);
}

void
_debug(const char * format, ...)
{
	static int initialized = 0;
	if (!(initialized++))
		openlog(NULL, 0, LOG_AUTHPRIV);

	va_list ap;
	va_start(ap, format);
	vsyslog(LOG_DEBUG, format, ap);
	va_end(ap);
}

int
pam_sm_authenticate(pam_handle_t *pamh, int flags, int argc, const char **argv)
{
	int pam_err = PAM_SUCCESS;

	/*
	 * Ask the user for their access token
	 */
	const char * access_token = NULL;
	pam_err = _get_client_access_token(pamh, &access_token);
	if (pam_err != PAM_SUCCESS)
		goto cleanup;

	/*
	 * Introspect the token 
	 */
	struct auth_introspect * introspect = NULL;
	struct auth_error * ae = auth_introspect_token(SSHServiceID, 
	                                               SSHServiceSecret,
	                                               access_token,
	                                               1,
	                                               &introspect);

	if (ae)
	{
		_debug("There should be a message here!!");
		_debug(ae->error_message);
		auth_free_error(ae);
		pam_err = PAM_SYSTEM_ERR;
		goto cleanup;
	}

	/*
	 * Check the validity of the token
	 */
	if (!introspect->active)
	{
		_debug("token is not active");
		pam_err = PAM_AUTH_ERR;
		goto cleanup;
	}

	/*
	 * Check for the appropriate scope(s)
	 */
	if (!_has_scope(SSHScope, (const char **)introspect->scopes))
	{
		_debug("token does not have proper scope");
		pam_err = PAM_AUTH_ERR;
		goto cleanup;
	}

	/*
	 * Map remote identities to local accounts
	 */
	char ** mapped_local_accounts = acct_map_id_to_accts((const char **)introspect->identities);

	/*
	 * Get the requested local account
	 */
	const char * requested_account = NULL;
	pam_err = _get_requested_account(pamh, &requested_account);
	if (pam_err != PAM_SUCCESS)
		goto cleanup;

	/*
	 * If requested local account is 'globus-mapping', print mapped accounts and exit
	 * XXX Is this a security risk?
	 */
	if (strcmp(requested_account, "globus-mapping") == 0)
	{
		pam_err = _display_valid_accounts(pamh, (const char * const *)mapped_local_accounts);
		if (pam_err == PAM_SUCCESS)
			pam_err = PAM_MAXTRIES;
		goto cleanup;
	}

	/*
	 * If requested local account is in mapped accounts, success. XXX we should display a message.
	 * Is this a security risk?
	 * Regardless of what we return there, if the account is not real, SSHD is going to retry us
	 * so that an attacker can not guess account names.
	 */
	if (!_is_string_in_list(requested_account, (const char * const *)mapped_local_accounts))
		pam_err = PAM_USER_UNKNOWN;

cleanup:
	if (access_token)
		free((void *)access_token);

	if (introspect)
		auth_free_introspect(introspect);

	if (mapped_local_accounts)
		acct_map_free_list(mapped_local_accounts);

	return pam_err;
}

int
pam_sm_setcred(pam_handle_t *pamh, int flags, int argc, const char **argv)
{
	if (flags & PAM_SILENT) {}
	if (flags & PAM_ESTABLISH_CRED) {}
	if (flags & PAM_DELETE_CRED) {}
	if (flags & PAM_REINITIALIZE_CRED) {}
	if (flags & PAM_REFRESH_CRED) {}
	return PAM_SUCCESS;
}

