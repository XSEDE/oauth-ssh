/*
 * Local includes.
 */
#include "credentials.h"

struct credentials
init_bearer_creds(const char * bearer_token)
{
	struct credentials creds = {.type    = BEARER,
	                            .u.token = bearer_token};
	return creds;
}

struct credentials
init_client_creds(const char * client_id, const char * client_secret)
{
	struct credentials creds = {.type            = CLIENT,
	                            .u.client.id     = client_id,
	                            .u.client.secret = client_secret};
	return creds;
}

