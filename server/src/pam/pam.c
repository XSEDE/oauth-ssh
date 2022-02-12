/*
 * System includes.
 */
#define PAM_SM_AUTH
#include <security/pam_modules.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <pwd.h>

/*
 * Local includes.
 */
#include "account_map.h"
#include "globus_auth.h"
#include "identities.h"
#include "introspect.h"
#include "strings.h"
#include "client.h"
#include "config.h"
#include "logger.h"
#include "base64.h"
#include "debug.h" // always last

#ifdef WITH_SCITOKENS
#include "scitokens_verify.h"
#endif // WITH_SCITOKENS

typedef int pam_status_t;

static bool
_acct_is_valid(const char * acct)
{
	return (getpwnam(acct) != NULL);
}

static char **
_build_account_array(const struct account_map * account_map)
{
	char ** account_array = NULL;

	for (const struct account_map * tmp = account_map; tmp; tmp = tmp->next)
	{
		for (int i = 0; tmp->accounts && tmp->accounts[i]; i++)
		{
			if (!key_in_list(CONST(char *, account_array), tmp->accounts[i]))
			{
				if (_acct_is_valid(tmp->accounts[i]))
					insert(&account_array, tmp->accounts[i]);
			}
		}
	}
	return account_array;
}

static char *
_build_json_list(const char * const * array)
{
	char * jlist = NULL;
	for (int i = 0; array && array[i]; i++)
	{
		if (jlist)
			append(&jlist, ", ");
		append(&jlist, "\"");
		append(&jlist, array[i]);
		append(&jlist, "\"");
	}
	return jlist;
}

static char *
_build_error_reply(const char * code, const char * description)
{
	return sformat("{"
	                   "\"error\": {"
	                       "\"code\": \"%s\","
	                       "\"description\": \"%s\""
	                   "}"
	               "}",
	               code,
	               description);
}

static struct pam_conv *
_get_pam_conv(pam_handle_t * pam)
{
	struct pam_conv * conv = NULL;
	int pam_err = pam_get_item(pam, PAM_CONV, (const void **)&conv);
	if (pam_err != PAM_SUCCESS)
	{
		logger(LOG_TYPE_ERROR,
		       "Failed to retrieve PAM conversation function: %s",
		       pam_strerror(pam, pam_err));
 		return NULL;
	}
	return conv;
}

static char *
_read_user_request(pam_handle_t * pam)
{
	const char * prompt = "Enter your OAuth token: ";

	int pam_err = PAM_SUCCESS;
	struct pam_message msg;
	const struct pam_message * msgp;
	struct pam_response      * resp;

	struct pam_conv * conv = _get_pam_conv(pam);
	if (!conv) return NULL;

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

	if (pam_err != PAM_SUCCESS || !resp || !resp->resp)
	{
		logger(LOG_TYPE_ERROR,
		       "Failed to read user input: %s",
		       pam_strerror(pam, pam_err));
		return NULL;
	}

	char * token = resp->resp;
	free(resp);
	return token;
}

static void
_send_our_reply(pam_handle_t * pam, const char * reply)
{
	if (reply)
	{
		struct pam_conv * conv = _get_pam_conv(pam);
		if (!conv) return;

		struct pam_message msg;
		msg.msg_style = PAM_TEXT_INFO;
		msg.msg       = reply;
		const struct pam_message * msgp = &msg;
		struct pam_response      * resp = NULL;
		int pam_err   = (*conv->conv)(1, &msgp, &resp, conv->appdata_ptr);

		if (pam_err != PAM_SUCCESS)
			logger(LOG_TYPE_ERROR,
			       "Failed to send our reply: %s",
			       pam_strerror(pam, pam_err));

		if (resp) free(resp->resp);
		free(resp);
	}
}

bool
_has_scope(char * fqdns[], const char * scopes, const char * suffix)
{
	if (!scopes) return false;
	if (!fqdns)  return false;

	bool found = false;

	char * str  = strdup(scopes);
	char * tmp  = str;
	char * sptr = NULL;

	// For each scope...
	char * scope = NULL;
	while ((scope = strtok_r(str, " ", &sptr)))
	{
		str = NULL;

		// Check the prefix
		const char * prefix = "https://auth.globus.org/scopes/";
		if (strncmp(scope, prefix, strlen(prefix)))
			continue;

		const char * slash = strchr(scope+strlen(prefix), '/');
		if (!slash) continue;

		// Check the suffix
		if (strcmp(slash+1, suffix))
			continue;

		// Check the FQDN
		for (int i = 0; fqdns[i]; i++)
		{
			// Check that the length of the FQDN's match
			if (strlen(fqdns[i]) != (slash - (scope+(strlen(prefix)))))
				continue;

			// Check that the FQDN's match
			if (strncmp(fqdns[i], scope+strlen(prefix), strlen(fqdns[i])))
				continue;

			// Found it
			found = true;
			goto cleanup;
		}
	}

	cleanup:
	free(tmp);
	return found;
}

bool
_is_token_valid(const struct introspect * introspect,
                const struct client     * client)
{
	if (introspect->active == false)
		return false;

	if (introspect->exp < time(NULL))
		return false;

	// This would indicate time skew on the local system
	if (introspect->iat > time(NULL))
		return false;

	if (introspect->nbf > time(NULL))
		return false;

	if (!_has_scope(client->fqdns, introspect->scope, "ssh"))
		return false;

	return true;
}

static const struct identity_provider *
_lookup_idp(const struct identities * identities, const char * idp_uuid)
{
	// Should never happen
	if (!identities->included.identity_providers)
		return NULL;

	for (int i = 0; identities->included.identity_providers[i]; i++)
	{
		// Shortcut
		const struct identity_provider * identity_provider = NULL;
		identity_provider = identities->included.identity_providers[i];

		// config specified an IdP UUID
		if (strcmp(identity_provider->id, idp_uuid) == 0)
			return identity_provider;
	}

	// Should never happen
	return NULL;
}

static bool
_is_session_valid(const struct config     * config,
                  const struct introspect * introspect,
                  const struct identities * identities)
{
	/*
	 * Security Policy Enforcement
	 */

	if (!config->permitted_idps && !config->authentication_timeout && !config->mfa)
		return true;

	if (!introspect->session_info || !introspect->session_info->authentications)
		return false;

	for (int k = 0; introspect->session_info->authentications[k]; k++)
	{
		// Shortcut to the authentication structure
		const struct authentication * authentication = NULL;
		authentication = introspect->session_info->authentications[k];

		if (config->authentication_timeout)
		{
			time_t seconds_since_auth = time(NULL) - authentication->auth_time;
			if (seconds_since_auth > (60*config->authentication_timeout))
				continue;
		}

		if (config->mfa)
		{
			if (!authentication->amr.mfa)
			    continue;
		}

		// Recent authentication with no IdP requirement
		if (!config->permitted_idps)
			return true;

		// Short cut to the idp used in this authentication
		const struct identity_provider * authed_idp = NULL;
		authed_idp = _lookup_idp(identities, authentication->idp);

		// This should not happen
		ASSERT(authed_idp);

		if (authed_idp)
		{
			for (int i = 0; config->permitted_idps[i]; i++)
			{
				// Match by IdP UUID
				if (strcmp(config->permitted_idps[i], authed_idp->id) == 0)
					return true;

				// Match by IdP domain
				if (key_in_list(CONST(char *, authed_idp->domains),
				                config->permitted_idps[i]))
					return true;
			}
		}
	}
	return false;
}

static pam_status_t
_cmd_get_security_policy(struct config * config, char ** reply)
{
	char * idp_list = _build_json_list(CONST(char *,config->permitted_idps));
	char * sidps = sformat("%s%s%s", idp_list?"[":"",
	                                 idp_list?idp_list:"null",
	                                 idp_list?"]":"");
	char * stmp = sformat("%d", config->authentication_timeout);
	char * stimeout = sformat("%s", config->authentication_timeout?stmp:"null");
	char * rformat = "{"
	                      "\"policy\": {"
	                          "\"permitted_idps\": %s,"
	                          "\"authentication_timeout\": %s"
	                      "}"
	                 "}";

	*reply = sformat(rformat, sidps, stimeout);

	free(idp_list);
	free(sidps);
	free(stmp);
	free(stimeout);
	return PAM_MAXTRIES;
}

static pam_status_t
_cmd_get_account_map(struct config  * config,
                     const char     * access_token,
                     char          ** reply)
{
	struct client     * client     = NULL;
	struct introspect * introspect = NULL;
	struct identities * identities = NULL;
	struct account_map * account_map = NULL;

	*reply = NULL;

	pam_status_t pam_status = PAM_AUTHINFO_UNAVAIL;
	introspect = get_introspect_resource(config, access_token);
	if (!introspect)
	{
		*reply = _build_error_reply("UNEXPECTED_ERROR",
		                            "An unexpected error occurred.");
		goto cleanup;
	}

	client = get_client_resource(config);
	if (!client)
	{
		*reply = _build_error_reply("UNEXPECTED_ERROR",
		                            "An unexpected error occurred.");
		goto cleanup;
	}

	if (!_is_token_valid(introspect, client))
	{
		*reply = _build_error_reply("INVALID_TOKEN", "Invalid token.");
		pam_status = PAM_AUTH_ERR;
		goto cleanup;
	}

	pam_status = PAM_AUTHINFO_UNAVAIL;
	identities = get_identities_resource(config, introspect);
	if (!identities) goto cleanup;

	if (!_is_session_valid(config, introspect, identities))
	{
		*reply = _build_error_reply("SESSION_VIOLATION", "The access token does not meet session requirements.");
		pam_status = PAM_AUTH_ERR;
		goto cleanup;
	}

	account_map = account_map_init(config, identities);

	char ** acct_array = _build_account_array(account_map);
	char *  acct_list  = _build_json_list(CONST(char *,acct_array));
	char * format = "{"
	                    "\"account_map\": {"
	                        "\"permitted_accounts\": [%s]"
	                    "}"
	                "}";

	*reply = sformat(format, acct_list?acct_list:"");

	free_array(acct_array);
	free(acct_list);

	pam_status = PAM_MAXTRIES;

cleanup:
	client_fini(client);
	introspect_fini(introspect);
	identities_fini(identities);
	account_map_fini(account_map);
	return pam_status;
}

/*
 * Given an access token, determine if the string is _most_ _likely_ a SciToken
 * as opposed to a Globus Auth token. Returns:
 *   true - token is _probably_ a SciToken
 *   false - token is not a SciToken
 *
 * This is a quick and dirty check; we could attempt to decode the JWT token as
 * a JWT and return true on success, but this should be sufficient for telling
 * Globus Auth tokens from SciTokens.
 */
static bool
_is_likely_a_scitoken(const char * token)
{
	/*
	 * SciTokens are JWTs, so they are base64 url encoded with '.' as
	 * delmiters for each part. Globus Auth tokens use the base64 character
	 * set (but not really base64-encoded because they are opaque). So here
	 * we look for any character in the JWT character set that is outside of
	 * the base64 url encode character set.
	 */
	return (strchr(token, '.') != NULL);
}

static pam_status_t
_cmd_login_scitokens(pam_handle_t   * pam,
                     struct config  * config,
                     const char     * access_token)
{
	pam_status_t   pam_status     = PAM_AUTHINFO_UNAVAIL;
	const char   * requested_user = NULL;
	pam_get_user(pam, &requested_user, NULL);

	// Handle the case where the module is not configured for SciTokens
	if (!config_auth_method(config, SCITOKENS))
	{
		logger(LOG_TYPE_INFO,
		       "SciToken received but this module is not configured for SciToken authorization.");
		pam_status = PAM_AUTHINFO_UNAVAIL;
		goto cleanup;
	}

#ifdef WITH_SCITOKENS
	if(scitoken_verify(access_token, config, requested_user))
	{
		logger(LOG_TYPE_INFO,
		       "Scitoken Identity %s authorizing as a local user",
		       requested_user);
		pam_status = PAM_SUCCESS;
		goto cleanup;
	}
#endif //SCITOKENS

	// SciTokens authorization has failed
	pam_status = PAM_AUTH_ERR;

cleanup:
	return pam_status;
}

static pam_status_t
_cmd_login_globus(pam_handle_t   * pam,
                  struct config  * config,
                  const char     * access_token,
                  char          ** reply)
{
	struct client      * client         = NULL;
	struct introspect  * introspect     = NULL;
	struct identities  * identities     = NULL;
	struct account_map * account_map    = NULL;
	pam_status_t         pam_status     = PAM_AUTHINFO_UNAVAIL;
	const char         * requested_user = NULL;

	pam_get_user(pam, &requested_user, NULL);
	*reply = NULL;

	// Handle the case where the module is not configured for Globus authorization.
	if (!config_auth_method(config, GLOBUS_AUTH))
	{
		logger(LOG_TYPE_INFO,
		       "Globus Auth token received but this module is not configured for Globus authorization.");
		pam_status = PAM_AUTHINFO_UNAVAIL;
		goto cleanup;
	}

	introspect = get_introspect_resource(config, access_token);
	if (!introspect)
	{
		*reply = _build_error_reply("UNEXPECTED_ERROR", "An unexpected error occurred.");
		pam_status = PAM_AUTHINFO_UNAVAIL;
		goto cleanup;
	}

	client = get_client_resource(config);
	if (!client)
	{
		*reply = _build_error_reply("UNEXPECTED_ERROR", "An unexpected error occurred.");
		pam_status = PAM_AUTHINFO_UNAVAIL;
		goto cleanup;
	}

	if (!_is_token_valid(introspect, client))
	{
		*reply = _build_error_reply("INVALID_TOKEN", "Invalid token.");
		pam_status = PAM_AUTH_ERR;
		goto cleanup;
	}

	identities = get_identities_resource(config, introspect);
	if (!identities)
	{
		*reply = _build_error_reply("UNEXPECTED_ERROR", "An unexpected error occurred.");
		pam_status = PAM_AUTHINFO_UNAVAIL;
		goto cleanup;
	}

	if (!_is_session_valid(config, introspect, identities))
	{
		char * tmp = _build_error_reply("SESSION_VIOLATION", "The access token does not meet session requirements.");
		*reply = base64_encode(tmp);
		free(tmp);
		pam_status = PAM_AUTH_ERR;
		goto cleanup;
	}

	account_map = account_map_init(config, identities);

	if (!is_acct_in_map(account_map, requested_user))
	{
		pam_status = PAM_AUTH_ERR;
		*reply = _build_error_reply("INVALID_ACCOUNT", "You cannot use that local account.");
		goto cleanup;
	}

	logger(LOG_TYPE_INFO,
	       "Identity %s authorized as local user %s",
	       acct_to_username(account_map, requested_user),
	       requested_user);

	pam_status = PAM_SUCCESS;

cleanup:
	client_fini(client);
	introspect_fini(introspect);
	identities_fini(identities);
	account_map_fini(account_map);

	return pam_status;
}


static pam_status_t
_cmd_login(pam_handle_t   * pam,
           struct config  * config,
           const char     * access_token,
           char          ** reply)
{
	*reply = NULL;

	/*
	 * Try to route the access token to the correct auth mechanism. We do
	 * not try all enabled auth methods for each token because the mechanism
	 * response to an invalid token vs the mechanisms response to an unknown
	 * token may not be differentiable. Also, that would be slower and make
	 * logging and error reporting more complicated.
	 */
	if (_is_likely_a_scitoken(access_token))
		return _cmd_login_scitokens(pam, config, access_token);
	return _cmd_login_globus(pam, config, access_token, reply);
}


/*
 * This is the 'cut-n-paste your access token' method
 */
static pam_status_t
_cmd_login_fallback(pam_handle_t  * pam,
                    struct config * config,
                    const char    * access_token)
{
	char * reply = NULL;
	pam_status_t pam_status = _cmd_login(pam, config, access_token, &reply);
	free(reply);
	return pam_status;
}

/*
 * Breakdown the values given at our passphrase prompt.
 */
static void
_decode_input(const char * user_input,
              char      ** op,
              char      ** access_token)
{
	char * decoded_input = NULL;
	jobj_t * jobj = NULL;

	*op = NULL;
	*access_token = NULL;

	decoded_input = base64_decode(user_input);
	if (!decoded_input)
		goto cleanup;

	jobj = jobj_init(decoded_input, NULL);
	if (!jobj)
		goto cleanup;

	jobj_t * j_cmd = jobj_get_value(jobj, "command");
	if (!j_cmd)
		goto cleanup;

	const char * tmp = jobj_get_string(j_cmd, "op");
	if (tmp) *op = strdup(tmp);

	tmp = jobj_get_string(j_cmd, "access_token");
	if (tmp) *access_token = strdup(tmp);

cleanup:
	free(decoded_input);
	jobj_fini(jobj);
}

static pam_status_t
_process_command(pam_handle_t  * pam,
                 struct config * config,
                 const char    * user_input,
                 char         ** reply)
{
	pam_status_t pam_status = PAM_AUTHINFO_UNAVAIL;

	char * op = NULL;
	char * access_token = NULL;

	_decode_input(user_input, &op, &access_token);

	if (op)
	{
		logger(LOG_TYPE_DEBUG, "OP: %s", op);

		if (strcmp(op, "get_security_policy") == 0)
		{
			pam_status = _cmd_get_security_policy(config, reply);
		}
		else if (strcmp(op, "get_account_map") == 0 && access_token)
		{
			pam_status = _cmd_get_account_map(config, access_token, reply);
		}
		else if (strcmp(op, "login") == 0 && access_token)
		{
			pam_status = _cmd_login(pam, config, access_token, reply);
		} else
		{
			char * tmp = _build_error_reply("UNKNOWN_COMMAND",
			                                "Unknown command.");
			*reply = base64_encode(tmp);
			free(tmp);
		}

		logger(LOG_TYPE_DEBUG, "REPLY: %s", *reply ? *reply : "NONE");

		if (*reply)
		{
			char * encoded_reply = base64_encode(*reply);
			free(*reply);
			*reply = encoded_reply;
		}
	}
	else
	{
		pam_status = _cmd_login_fallback(pam, config, user_input);
	}

	return pam_status;
}

pam_status_t
pam_sm_authenticate(pam_handle_t *pam, int flags, int argc, const char **argv)
{
	struct config * config     = NULL;
	char          * user_input = NULL;
	char          * reply      = NULL;
	pam_status_t    pam_status = PAM_AUTHINFO_UNAVAIL;

	(void) logger_init(flags, argc, argv);

	config = config_init(flags, argc, argv);
	if (!config) goto cleanup;

	user_input = _read_user_request(pam);
	if (!user_input) goto cleanup;

	pam_status = _process_command(pam, config, user_input, &reply);
	_send_our_reply(pam, reply);

cleanup:
	config_fini(config);
	free(reply);
	free(user_input);
	return pam_status;
}

int
pam_sm_setcred(pam_handle_t *pam, int flags, int argc, const char **argv)
{
	if (flags & PAM_SILENT) {}
	if (flags & PAM_ESTABLISH_CRED) {}
	if (flags & PAM_DELETE_CRED) {}
	if (flags & PAM_REINITIALIZE_CRED) {}
	if (flags & PAM_REFRESH_CRED) {}
	return PAM_SUCCESS;
}
