/*
 * Copyright The University of Chicago
 * All Rights Reserved.
 */

/*
 * System includes.
 */
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <syslog.h>
#define PAM_SM_AUTH
#include <security/pam_modules.h>

/*
 * Local includes.
 */
#include "acct_map_module_file.h"
#include "acct_map_module_idp.h"
#include "credentials.h"
#include "introspect.h"
#include "identities.h"
#include "pam_utils.h"
#include "acct_map.h"
#include "strings.h"
#include "config.h"
#include "scope.h"

static const char * AccessTokenPrompt = "Enter your Globus Auth token: ";
static const char * NoLocalAcctMsg        = "You have no local accounts";
static const char * LocalAcctMsgPrefix    = "You can log in as";
static const char * DefaultConfigFile     = "/etc/globus/globus-ssh.conf";
static const char * ConfigOptClientID     = "auth_client_id";
static const char * ConfigOptClientSecret = "auth_client_secret";
static const char * ConfigOptIdpSuffix    = "idp_suffix";
static const char * ConfigOptMapFile      = "map_file";
static const char * ScopeSuffix           = "ssh";


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

int
get_client_access_token(struct pam_conv *  conv,
                        const char      *  prompt,
                        char            ** access_token);

int
display_client_message(struct pam_conv * conv,
                       const char      * message);

char *
build_acct_map_msg(const char * const accounts[]);

static int
lookup_accounts(const char      *   idp_suffix,
                const char      *   map_file,
                struct identity *   identities[],
                char            *** accounts);

static void
debug(const char * format, ...)
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
	char * access_token      = NULL;
	char * client_id         = NULL;
	char * client_secret     = NULL;
	char * idp_suffix        = NULL;
	char * map_file          = NULL;
	char * error_msg         = NULL;
	struct identity **  identities = NULL;
	struct introspect * introspect_record = NULL;

	int pam_err = PAM_SUCCESS;

	/*
	 * Default config options.
	 */
	const char * config_file = DefaultConfigFile;

	/*
	 * Parse provided options.
	 */
	if (argc >= 1) config_file = argv[0];

	/*
	 * Load our config
	 */
	struct config * config = config_init();

	int retval = config_load(config, config_file);
	if (retval < 0)
	{
		debug("COULD NOT READ %s: %s", config_file, strerror(-retval));
		pam_err  = PAM_AUTHINFO_UNAVAIL;
		goto cleanup;
	}

	if (retval > 0)
	{
		debug("COULD NOT PARSE %s LINE %d", config_file, retval);
		pam_err  = PAM_AUTHINFO_UNAVAIL;
		goto cleanup;
	}

	config_get_value(config, ConfigOptClientID, &client_id);
	if (!client_id)
	{
		debug("%s IS MISSING FROM %s", ConfigOptClientID, config_file);
		pam_err  = PAM_AUTHINFO_UNAVAIL;
		goto cleanup;
	}

	config_get_value(config, ConfigOptClientSecret, &client_secret);
	if (!client_secret)
	{
		debug("%s IS MISSING FROM %s", ConfigOptClientSecret, config_file);
		pam_err  = PAM_AUTHINFO_UNAVAIL;
		goto cleanup;
	}

	struct credentials creds = init_client_creds(client_id, client_secret);

	config_get_value(config, ConfigOptIdpSuffix, &idp_suffix);
	config_get_value(config, ConfigOptMapFile, &map_file);

	if (!idp_suffix && !map_file)
	{
		debug("BOTH %s and %s ARE MISSING FROM %s", ConfigOptIdpSuffix,
		                                             ConfigOptMapFile,
		                                             config_file);
		pam_err  = PAM_AUTHINFO_UNAVAIL;
		goto cleanup;
	}

	struct pam_conv * conv = NULL;
	pam_err = pam_get_item(pamh, PAM_CONV, (const void **)&conv);
	if (pam_err != PAM_SUCCESS)
	{
		pam_err = PAM_AUTH_ERR;
		return pam_err;
	}

	/*
	 * Ask the user for their access token
	 */
	pam_err = get_client_access_token(conv, AccessTokenPrompt, &access_token);
	if (pam_err != PAM_SUCCESS)
		goto cleanup;

	/*
	 * Introspect the token 
	 */
	retval = introspect(&creds, access_token, &introspect_record, &error_msg);
	if (retval)
	{
		debug(error_msg);
		pam_err = PAM_AUTH_ERR;
		goto cleanup;
	}

	/*
	 * Check the validity of the token
	 */
	if (!introspect_record->active)
	{
		debug("token is not active");
		pam_err = PAM_AUTH_ERR;
		goto cleanup;
	}

	/*
	 * Check for the appropriate scope(s)
	 */
	int found = has_scope(ScopeSuffix, (char **)introspect_record->scopes, &error_msg);
	if (error_msg || !found)
	{
		switch (error_msg == NULL)
		{
		case 0:
			debug("error while searching for the proper scope: %s", error_msg);
			free(error_msg);
			break;
		case 1:
			debug("token does not have proper scope");
			break;
		}

		pam_err = PAM_AUTH_ERR;
		goto cleanup;
	}

	/*
	 * Map remote identities to local accounts
	 */
	retval = get_identities(&creds,
	                      (const char **)introspect_record->identities,
                           &identities,
                           &error_msg);

	if (retval)
	{
		debug(error_msg);
		pam_err = PAM_AUTH_ERR;
		goto cleanup;
	}

	char ** local_accts;
	pam_err = lookup_accounts(idp_suffix, map_file, identities, &local_accts);
	if (pam_err != PAM_SUCCESS)
			goto cleanup;

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
		const char * const * tmp = (const char * const *) local_accts;
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
	if (!string_in_list(requested_account, (const char * const *)local_accts))
		pam_err = PAM_USER_UNKNOWN;

cleanup:
	if (config)
		config_free(config);
	if (access_token)
		free((void *)access_token);
	if (client_id)
		free(client_id);
	if (client_secret)
		free(client_secret);
	if (idp_suffix)
		free(idp_suffix);
	if (map_file)
		free(map_file);
	if (introspect_record)
		free_introspect(introspect_record);
	if (identities)
		free_identities(identities);

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
 *  * Send a single message, type text info, display the given prompt.
 *   */
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
build_acct_map_msg(const char * const accounts[])
{
	char * account_list = join(accounts, ',');
	if (!account_list)
		return strdup(NoLocalAcctMsg);

	const char * const list[] = {LocalAcctMsgPrefix, account_list, NULL};

	char * acct_map_msg = join(list, ' ');

	free (account_list);
	return acct_map_msg;
}

static int
lookup_accounts(const char      *   idp_suffix,
                const char      *   map_file,
                struct identity *   identities[],
                char            *** accounts)
{
	*accounts = NULL;

	struct acct_map * acct_map = acct_map_init();

	int retval = 0;
	if (idp_suffix)
	{
		retval = acct_map_add_module(acct_map, AcctMapModuleIdpSuffix, idp_suffix);
		if (retval)
			goto cleanup;
	}

	if (map_file)
	{
		retval = acct_map_add_module(acct_map, AcctMapModuleMapFile, map_file);
		if (retval)
			goto cleanup;
	}

	char ** ids = calloc((2*get_array_len((void**)identities)) + 1, sizeof(char *));
	for (int i = 0; identities[i]; i++)
	{
		ids[2*i] = identities[i]->id;
		ids[(2*i)+1] = identities[i]->username;
	}

	*accounts = acct_map_lookup(acct_map, ids);

cleanup:
	if (ids)
		free(ids);
	if (acct_map)
		acct_map_free(acct_map);
	return retval == 0 ? PAM_SUCCESS : PAM_AUTH_ERR;
}
