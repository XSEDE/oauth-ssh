#include <string.h>
#include <json-c/json.h>
#include "auth_introspect.h"
#include "strings.h"
#include "json.h"

#include "test/unit_test.h"

struct auth_introspect *
_auth_introspect_from_json_str(const char * j_string)
{
	if (!j_string) return NULL;

	struct json_tokener    * j_tokener = json_tokener_new();
	struct json_object     * j_obj = json_tokener_parse_ex(j_tokener, j_string, strlen(j_string));
	struct auth_introspect * i = calloc(sizeof(*i), 1);

	i->active = _json_to_bool(j_obj, "active");

	if (i->active)
	{
		char * scope_string = _json_to_string(j_obj, "scope");

		i->scopes       = _strings_split_to_array(scope_string);
		i->expiry       = _json_to_int(j_obj,          "exp");
		i->issued_at    = _json_to_int(j_obj,          "iat");
		i->not_before   = _json_to_int(j_obj,          "nbf");
		i->client_id    = _json_to_string(j_obj,       "client_id");
		i->sub          = _json_to_string(j_obj,       "sub");
		i->username     = _json_to_string(j_obj,       "username");
		i->display_name = _json_to_string(j_obj,       "name");
		i->email        = _json_to_string(j_obj,       "email");
		i->issuer       = _json_to_string(j_obj,       "iss");
		i->audiences    = _json_to_string_array(j_obj, "aud");
		i->identities   = _json_to_string_array(j_obj, "identities_set");

		if (scope_string) free(scope_string);
	}

        json_tokener_free(j_tokener);
	return i;
}

void
_auth_introspect_free(struct auth_introspect * i)
{
	if (i)
	{
		if (i->client_id)    free(i->client_id);
		if (i->sub)          free(i->sub);
		if (i->username)     free(i->username);
		if (i->display_name) free(i->display_name);
		if (i->email)        free(i->email);
		if (i->issuer)       free(i->issuer);

		for (int j = 0; i->audiences && i->audiences[j]; j++)
		{
			free(i->audiences[j]);
		}
		if (i->audiences) free(i->audiences);

		for (int j = 0; i->scopes && i->scopes[j]; j++)
		{
			free(i->scopes[j]);
		}
		if (i->scopes) free(i->scopes);

		for (int j = 0; i->identities && i->identities[j]; j++)
		{
			free(i->identities[j]);
		}
		if (i->identities) free(i->identities);

		free(i);
	}
}
