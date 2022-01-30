/*
 * System includes.
 */
#include <string.h>

/*
 * Local includes.
 */
#include "globus_auth.h"
#include "strings.h"
#include "logger.h"
#include "http.h"
#include "json.h"
#include "debug.h" // always last


const char *
globus_auth_host(const struct config * config)
{
	if (config->environment)
	{
		if (strcasecmp(config->environment, "preview") == 0)
			return "auth.preview.globus.org";

		if (strcasecmp(config->environment, "staging") == 0)
			return "auth.staging.globuscs.info";

		if (strcasecmp(config->environment, "test") == 0)
			return "auth.test.globuscs.info";

		if (strcasecmp(config->environment, "integration") == 0)
			return "auth.integration.globuscs.info";

		if (strcasecmp(config->environment, "sandbox") == 0)
			return "auth.sandbox.globuscs.info";
	}
	return "auth.globus.org";
}

struct introspect *
get_introspect_resource(const struct config * config, const char * token)
{
	char * reply_body = NULL;
	char * error_msg  = NULL;
	json_t * json = NULL;
	struct introspect * introspect = NULL;

	// construct request url
	const char * host = globus_auth_host(config);
	char * request_url = sformat("https://%s/v2/oauth2/token/introspect", host);

	// construct request body
	const char * body_format = "token=%s&include=identities_set,session_info";
	char * request_body = sformat(body_format, token);

	if (http_post_request(config, request_url, request_body, &reply_body))
		goto cleanup;

	if ((json = jobj_init(reply_body, &error_msg)))
	{
		if (!jobj_key_exists(json, "errors"))
			introspect = introspect_init(json);
	}

	if (!introspect)
	{
		logger(LOG_TYPE_ERROR,
		       "An error occurred while talking with Globus Auth%s",
		       reply_body);
		goto cleanup;
	}

	// Verify that the response includes a non empty identities_set. The
	// field is optional so the parser won't care if its missing.
	if (!introspect->identities_set || !introspect->identities_set[0])
	{
		logger(LOG_TYPE_ERROR, "The introspect response is missing the 'identities_set'.");
		introspect_fini(introspect);
		introspect = NULL;
		goto cleanup;
	}

	// Verify that the response includes a session_info. The field is
	// optional so the parser won't care if its missing.
	if (!introspect->session_info)
	{
		logger(LOG_TYPE_ERROR, "The introspect response is missing 'session_info'.");
		introspect_fini(introspect);
		introspect = NULL;
		goto cleanup;
	}

cleanup:
	free(request_body);
	free(request_url);
	free(reply_body);
	free(error_msg);
	jobj_fini(json);
	return introspect;
}

char *
build_id_list(const struct introspect * introspect)
{
	char * list = NULL;
	for (int i = 0; introspect->identities_set[i]; i++)
	{
		if (list)
			append(&list, ",");
		append(&list, introspect->identities_set[i]);
	}

	return list;
}

struct client *
get_client_resource(const struct config * config)
{
	char * reply_body = NULL;
	char * error_msg  = NULL;
	json_t * json = NULL;
	struct client * client = NULL;

	// construct request url
	const char * host = globus_auth_host(config);
	char * request_url = sformat(
	           "https://%s/v2/api/clients/%s",
	           host,
	           config->client_id);

	if (http_get_request(config, request_url, &reply_body))
		goto cleanup;

	if ((json = jobj_init(reply_body, &error_msg)))
	{
		if (!jobj_key_exists(json, "errors"))
			client = client_init(json);
	}

	if (!client)
	{
		logger(LOG_TYPE_ERROR,
		       "An error occurred while talking with Globus Auth%s",
		       reply_body);
		goto cleanup;
	}

cleanup:
	free(request_url);
	free(reply_body);
	free(error_msg);
	jobj_fini(json);
	return client;
}

struct identities *
get_identities_resource(const struct config * config,
                        const struct introspect * introspect)
{
	char * reply_body = NULL;
	char * error_msg  = NULL;
	json_t * json = NULL;
	struct identities * identities = NULL;

	// construct request url
	const char * host = globus_auth_host(config);
	char * id_list = build_id_list(introspect);
	char * request_url = sformat(
	           "https://%s/v2/api/identities?ids=%s&include=identity_provider",
	           host,
	           id_list);

	if (http_get_request(config, request_url, &reply_body))
		goto cleanup;

	if ((json = jobj_init(reply_body, &error_msg)))
	{
		if (!jobj_key_exists(json, "errors"))
			identities = identities_init(json);
	}

	if (!identities)
	{
		logger(LOG_TYPE_ERROR,
		       "An error occurred while talking with Globus Auth%s",
		       reply_body);
		goto cleanup;
	}
cleanup:
	free(id_list);
	free(request_url);
	free(reply_body);
	free(error_msg);
	jobj_fini(json);
	return identities;
}
