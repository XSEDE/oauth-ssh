/*
 * System includes.
 */
#include <stdlib.h>
#include <string.h>

/*
 * Local includes.
 */
#include "introspect.h"
#include "strings.h"
#include "http.h"
#include "auth.h"
#include "json.h"

static struct introspect * json_to_introspect(json_t * json);

int
introspect(struct credentials *  creds, 
           const char         *  token,
           struct introspect  ** introspect,
           char               ** error_msg)
{
	const char * list[]  = {"token=", token, "&include=identities_set", NULL};
	char * request = join(list, 0);
	char * reply   = NULL;

	int retval = http_post_request(creds,
	                               GLOBUS_AUTH_URL "/v2/oauth2/token/introspect",
	                               request,
	                               &reply,
	                               error_msg);

	if (retval)
		goto cleanup;

	json_t * json = json_init(reply, error_msg);
	if (!json)
		goto cleanup;

	*introspect = json_to_introspect(json);

cleanup:
	if (json)
		json_free(json);
	if (request)
		free(request);
	if (reply)
		free(reply);
	return retval || !json;
}

void
free_introspect(struct introspect * i)
{
	if (i->scopes)
	{
		for (int j = 0; i->scopes[j]; j++) free(i->scopes[j]);
		free(i->scopes);
	}

	if (i->sub)
		free(i->sub);

	if (i->username)
		free(i->username);

	if (i->display_name)
		free(i->display_name);

	if (i->email)
		free(i->email);

	if (i->client_id)
		free(i->client_id);

	if (i->audiences)
	{
		for (int j = 0; i->audiences[j]; j++) free(i->audiences[j]);
		free(i->audiences);
	}

	if (i->issuer)
		free(i->issuer);

	if (i->identities)
	{
		for (int j = 0; i->identities[j]; j++) free(i->identities[j]);
		free(i->identities);
	}

	free(i);
}

char **
copy_out_string_list(json_t * json, char * key)
{
	const char * string = json_get_string(json, key);
	if (!string)
		return NULL;
	return split(string, ' ');
}

char **
copy_out_array(json_t * json, char * key)
{
	json_t * j_arr = json_get_array(json, key);
	if (!j_arr)
		return NULL;

	if (json_array_len(j_arr) == 0)
		return NULL;

	char ** array = calloc((json_array_len(j_arr) + 1), sizeof(char *));

	for (int i = 0; i < json_array_len(j_arr); i++)
	{
		json_t * j_idx = json_array_idx(j_arr, i);
		array[i] = strdup(json_to_string(j_idx));
		json_free(j_idx);
	}
	json_free(j_arr);
	return array;
}

/* XXX we need to make sure the reply is not an error message.
 */

// https://docs.globus.org/api/auth/reference/#token_introspection_post_v2_oauth2_token_introspect
// XXX the web page is not clear on 'scopes' and 'aud', describing them as 'lists' however the
// example record are strings.
static struct introspect *
json_to_introspect(json_t * json)
{
	struct introspect * i = calloc(sizeof(struct introspect), 1);
	i->active       = json_get_bool(json, "active");
	i->scopes       = copy_out_string_list(json, "scope");
	i->sub          = safe_strdup(json_get_string(json, "sub"));
	i->username     = safe_strdup(json_get_string(json, "username"));
	i->display_name = safe_strdup(json_get_string(json, "name"));
	i->email        = safe_strdup(json_get_string(json, "email"));
	i->client_id    = safe_strdup(json_get_string(json, "client_id"));
	i->audiences    = copy_out_string_list(json, "aud");
	i->issuer       = safe_strdup(json_get_string(json, "iss"));
	i->expiry       = json_get_int(json, "exp");
	i->issued_at    = json_get_int(json, "iat");
	i->not_before   = json_get_int(json, "nbf");
	i->identities   = copy_out_array(json, "identities_set");

	return i;
}
