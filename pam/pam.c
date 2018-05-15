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
#include "utilities.h"
#include "constants.h"

extern const char * SSHServiceID;
extern const char * SSHServiceSecret;
static const char * SSHScope = "https://auth.globus.org/scopes/ssh.sandbox.globuscs.info/ssh";

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

	struct pam_conv * conv = NULL;
	pam_err = pam_get_item(pamh, PAM_CONV, (const void **)&conv);
	if (pam_err != PAM_SUCCESS)
		return pam_err;

	/*
	 * Ask the user for their access token
	 */
	char * access_token = NULL;
	pam_err = get_client_access_token(conv, AccessTokenPrompt, &access_token);
	if (pam_err != PAM_SUCCESS)
		goto cleanup;

	/*
	 * Introspect the token 
	 */
	struct auth_introspect * introspect = NULL;
	struct auth_error * error = auth_introspect_token(SSHServiceID, 
	                                                  SSHServiceSecret,
	                                                  access_token,
	                                                  1,
	                                                  &introspect);

	if (error)
	{
		_debug(error->error_message);
		auth_free_error(error);
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
	if (!has_scope(SSHScope, (const char **)introspect->scopes))
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
	pam_err = pam_get_user(pamh, &requested_account, NULL);
	if (pam_err != PAM_SUCCESS)
		goto cleanup;

	/*
	 * If requested local account is 'globus-mapping', print mapped accounts
	 * and exit
	 */
	if (strcmp(requested_account, "globus-mapping") == 0)
	{
		const char * const * tmp = (const char * const *) mapped_local_accounts;
		char * acct_map_msg = build_acct_map_msg(tmp);

		pam_err = display_client_message(conv, acct_map_msg);
		free(acct_map_msg);

		if (pam_err == PAM_SUCCESS)
			pam_err = PAM_MAXTRIES;
		goto cleanup;
	}

	/*
	 * If requested local account is in mapped accounts, success.
	 * Regardless of what we return there, if the account is not real,
	 * SSHD is going to retry us so that an attacker can not guess account
	 * names.
	 */
	if (!string_in_list(requested_account, (const char * const *)mapped_local_accounts))
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

