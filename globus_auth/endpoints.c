#include <stdlib.h>
#include <string.h>

#include "endpoints.h"
#include "request.h"
#include "strings.h"
#include "encode.h"
#include "json.h"

#include "tests/unit_test.h"

/*************************************************************************
 * Goals for this design:
 * 1) Breakout request creation so it is easily auditible against the
 *    Globus Auth API.
 * 2) Provide unit-testable request creation routines.
 ************************************************************************/

/************************************************
 * Move these to configure options. 
 ***********************************************/
//define GLOBUS_AUTH_URL "https://auth.globus.org"                // Production
//define GLOBUS_AUTH_URL "https://auth.preview.globus.org"        // Preview
//define GLOBUS_AUTH_URL "https://auth.staging.globuscs.info"     // Staging
//define GLOBUS_AUTH_URL "https://auth.test.globuscs.info"        // Test
//define GLOBUS_AUTH_URL "https://auth.integration.globuscs.info" // Integration
#define GLOBUS_AUTH_URL "https://auth.sandbox.globuscs.info"     // Sandbox


/************************************************
 *
 * /v2/oauth2/authorize
 *
 ***********************************************/

struct _request
_ep_authorize(const char      *  client_id, 
              const char      *  code_challenge, // Optional, native app only
              const char      *  redirect_uri,
              const char      ** scopes,
              const char      *  state,
              auth_access_type_t access_type)
{
	ASSERT(redirect_uri != NULL);
	ASSERT(client_id    != NULL);
	ASSERT(scopes       != NULL);
	ASSERT(state        != NULL);
	ASSERT(access_type >= 0 && access_type < AUTH_ACCESS_TYPE_MAX);

	char * access_type_str = access_type == AUTH_ACCESS_TYPE_ONLINE ? "online" : "offline";
	char * scope_list      = _strings_build_list(' ', scopes);

	struct _kv q_kv[] = {
	        _P_KV(_S_STR("response_type"),         _S_STR("code")),
	        _P_KV(_S_STR("client_id"),             _D_STR(client_id)),
	        _P_KV(_S_STR("redirect_uri"),          _D_STR(redirect_uri)),
	        _P_KV(_S_STR("scope"),                 _F_STR(scope_list)),
	        _P_KV(_S_STR("state"),                 _D_STR(state)),
	        _P_KV(_S_STR("access_type"),           _S_STR(access_type_str)),
	        _P_KV(_S_STR("code_challenge"),        _D_STR(code_challenge)),
	        _P_KV(_S_STR("code_challenge_method"), _S_STR("S256")),
	    };

	return _request_init(REQUEST_OP_GET, 
	                     _S_STR(GLOBUS_AUTH_URL"/v2/oauth2/authorize"),
	                     (struct _kvs){q_kv, sizeof(q_kv)/sizeof(struct _kv)},
	                     (struct _kvs){NULL, 0},
	                     NULL);
}

/************************************************
 *
 * /v2/oauth2/token
 *
 ***********************************************/

struct _request
_ep_token__code(const char * client_id,     // native app only
                const char * code_verifier, // native app only
                const char * redirect_uri,
                const char * authorization_code)
{
	ASSERT(redirect_uri != NULL);

	struct _kv b_kv[] = {
	        _P_KV(_S_STR("client_id"),     _D_STR(client_id)),
	        _S_KV(_S_STR("code"),          _D_STR(authorization_code)),
	        _P_KV(_S_STR("redirect_uri"),  _D_STR(redirect_uri)),
	        _S_KV(_S_STR("grant_type"),    _S_STR("authorization_code")),
	        _S_KV(_S_STR("code_verifier"), _D_STR(code_verifier)),
	    };

	return _request_init(REQUEST_OP_POST,
	                     _S_STR(GLOBUS_AUTH_URL"/v2/oauth2/token"),
	                     (struct _kvs){NULL, 0},
	                     (struct _kvs){b_kv, sizeof(b_kv)/sizeof(struct _kv)},
	                     NULL);
}
                 
struct _request
_ep_token__client(const char ** scopes)
{
	ASSERT(scopes != NULL && *scopes != NULL);
	char * scope_list = _strings_build_list('+', scopes);

	struct _kv b_kv[] = {
	        _P_KV(_S_STR("grant_type"), _S_STR("client_credentials")),
	        _P_KV(_S_STR("scope"),      _F_STR(scope_list)),
	    };

	return _request_init(REQUEST_OP_POST,
	                     _S_STR(GLOBUS_AUTH_URL"/v2/oauth2/token"),
	                     (struct _kvs){NULL, 0},
	                     (struct _kvs){b_kv, sizeof(b_kv)/sizeof(struct _kv)},
	                     NULL);
}

struct _request
_ep_token__dependent(const char       * access_token,
                     auth_access_type_t access_type)
{
	ASSERT(access_token != NULL);
	ASSERT(access_type >= 0 && access_type < AUTH_ACCESS_TYPE_MAX);

	char * access_type_str = NULL;

	switch (access_type)
	{
	case AUTH_ACCESS_TYPE_ONLINE:
		access_type_str = "online";
		break;
	case AUTH_ACCESS_TYPE_OFFLINE:
		access_type_str = "offline";
		break;
	}

	struct _kv b_kv[] = {
	        _P_KV(_S_STR("grant_type"), _S_STR("urn:globus:auth:grant_type:dependent_token")),
	        _P_KV(_S_STR("token"),      _D_STR(access_token)),
	        _P_KV(_S_STR("access_type"),_S_STR(access_type_str)),
	    };

	return _request_init(REQUEST_OP_POST,
	                     _S_STR(GLOBUS_AUTH_URL"/v2/oauth2/token"),
	                     (struct _kvs){NULL, 0},
	                     (struct _kvs){b_kv, sizeof(b_kv)/sizeof(struct _kv)},
	                     NULL);
}

struct _request
_ep_token__refresh(const char * native_client_id,  // Optional, native app only
                   const char * refresh_token)
{
	ASSERT(refresh_token != NULL);

	struct _kv b_kv[] = {
	        _P_KV(_S_STR("refresh_token"), _D_STR(refresh_token)),
	        _P_KV(_S_STR("client_id"),     _D_STR(native_client_id)),
	        _P_KV(_S_STR("grant_type"),    _S_STR("refresh_token")),
	    };

	return _request_init(REQUEST_OP_POST,
	                     _S_STR(GLOBUS_AUTH_URL"/v2/oauth2/token"),
	                     (struct _kvs){NULL, 0},
	                     (struct _kvs){b_kv, sizeof(b_kv)/sizeof(struct _kv)},
	                     NULL);
}

struct _request
_ep_token__revoke(const char * native_client_id, // Optional, native app only
                  const char * token)
{
	ASSERT(token != NULL);

	struct _kv b_kv[] = {
	        _P_KV(_S_STR("token"),     _D_STR(token)),
	        _P_KV(_S_STR("client_id"), _D_STR(native_client_id)),
	    };

	return _request_init(REQUEST_OP_POST,
	                     _S_STR(GLOBUS_AUTH_URL"/v2/oauth2/token/revoke"),
	                     (struct _kvs){NULL, 0},
	                     (struct _kvs){b_kv, sizeof(b_kv)/sizeof(struct _kv)},
	                     NULL);
}

struct _request
_ep_token__validate(const char * native_client_id, // Optional, native app only
                    const char * token)
{
	ASSERT(token != NULL);

	struct _kv b_kv[] = {
	        _P_KV(_S_STR("token"),     _D_STR(token)),
	        _P_KV(_S_STR("client_id"), _D_STR(native_client_id)),
	    };

	return _request_init(REQUEST_OP_POST,
	                     _S_STR(GLOBUS_AUTH_URL"/v2/oauth2/token/validate"),
	                     (struct _kvs){NULL, 0},
	                     (struct _kvs){b_kv, sizeof(b_kv)/sizeof(struct _kv)},
	                     NULL);
}

struct _request
_ep_token__introspect(const char * token, int include_id_set)
{
	ASSERT(token != NULL);

	char * value = NULL;
	if (include_id_set)
		value = "identities_set";

	struct _kv b_kv[] = {
	        _P_KV(_S_STR("token"),   _D_STR(token)),
	        _P_KV(_S_STR("include"), _S_STR(value)),
	    };

	return _request_init(REQUEST_OP_POST,
	                     _S_STR(GLOBUS_AUTH_URL"/v2/oauth2/token/introspect"),
	                     (struct _kvs){NULL, 0},
	                     (struct _kvs){b_kv, sizeof(b_kv)/sizeof(struct _kv)},
	                     NULL);
}

/************************************************
 *
 * /v2/oauth2/userinfo
 *
 ***********************************************/

struct _request
_ep_userinfo()
{
	return _request_init(REQUEST_OP_GET,
	                     _S_STR(GLOBUS_AUTH_URL"/v2/oauth2/userinfo"),
	                     (struct _kvs){NULL, 0},
	                     (struct _kvs){NULL, 0},
	                     NULL);
}

/************************************************
 *
 * /v2/api/identities
 *
 ***********************************************/

struct _request
_ep_identities__by_id(const char ** ids)
{
	ASSERT(ids != NULL && ids[0] != NULL);

	char * ids_list = _strings_build_list(',', ids);

	struct _kv q_kv[] = {
	        _P_KV(_S_STR("ids"), _F_STR(ids_list)),
	    };

	return _request_init(REQUEST_OP_GET, 
	                     _S_STR(GLOBUS_AUTH_URL"/v2/api/identities"),
	                     (struct _kvs){q_kv, sizeof(q_kv)/sizeof(struct _kv)},
	                     (struct _kvs){NULL, 0},
	                     NULL);
}

struct _request
_ep_identities__by_name(const char ** usernames)
{
	ASSERT(usernames != NULL && usernames[0] != NULL);

	char * user_list = _strings_build_list(',', usernames);

	struct _kv q_kv[] = {
	        _P_KV(_S_STR("usernames"), _F_STR(user_list)),
	    };

	return _request_init(REQUEST_OP_GET, 
	                     _S_STR(GLOBUS_AUTH_URL"/v2/api/identities"),
	                     (struct _kvs){q_kv, sizeof(q_kv)/sizeof(struct _kv)},
	                     (struct _kvs){NULL, 0},
	                     NULL);
}

/************************************************
 *
 * /v2/api/clients
 *
 ***********************************************/

// XXX where should this live?
// XXX this is not patchy
json_object *
_auth_client_to_json_str(const struct auth_client client)
{
	json_object * j_obj = _json_object_new();

	_json_object_add(j_obj, "public_client",    _json_bool(client.public_client));
	_json_object_add(j_obj, "name",             _json_string(client.name));
	_json_object_add(j_obj, "privacy_policy",   _json_string(client.privacy_policy));
	_json_object_add(j_obj, "preselect_idp",    _json_string(client.preselect_idp));
	_json_object_add(j_obj, "id",               _json_string(client.id));
	_json_object_add(j_obj, "parent_id",        _json_string(client.parent_id));
	_json_object_add(j_obj, "project_id",       _json_string(client.project_id));
	_json_object_add(j_obj, "terms_of_service", _json_string(client.terms_of_service));

	json_object * j_array = _json_array_new();
	for (int i=0; client.redirect_uris && client.redirect_uris[i]; i++)
	{
		_json_array_add(j_array, _json_string(client.redirect_uris[i]));
	}
	_json_object_add(j_obj, "redirect_uris", j_array);

	switch (client.visibility)
	{
	case CLIENT_VIS_PUBLIC:
		_json_object_add(j_obj, "visibility", _json_string("public"));
		break;
	case CLIENT_VIS_PRIVATE:
		_json_object_add(j_obj, "visibility", _json_string("private"));
		break;
#ifdef DEBUG
	default: ASSERT(0);
#endif
	}

	json_object * j_envelope = _json_object_new();
	_json_object_add(j_envelope, "client", j_obj);

	return j_envelope;
}

struct _request
_ep_clients__create(struct auth_client client)
{
	ASSERT(client.name);

	/*
	 * Mask unsupported fields.
	 */
	struct auth_client masked_client;
	memset(&masked_client, 0, sizeof(struct auth_client));
	masked_client.public_client    = client.public_client;
	masked_client.name             = client.name;
	masked_client.visibility       = client.visibility;
	masked_client.terms_of_service = client.terms_of_service;
	masked_client.privacy_policy   = client.privacy_policy;
	masked_client.required_idp     = client.required_idp;
	masked_client.preselect_idp    = client.preselect_idp;

	return _request_init(REQUEST_OP_POST, 
	                     _S_STR(GLOBUS_AUTH_URL"/v2/api/clients"),
	                     (struct _kvs){NULL, 0}, 
	                     (struct _kvs){NULL, 0}, 
	                     _auth_client_to_json_str(masked_client));
}

struct _request
_ep_clients__read_all()
{
	return _request_init(REQUEST_OP_GET, 
	                     _S_STR(GLOBUS_AUTH_URL"/v2/api/clients"),
	                     (struct _kvs){NULL, 0},
	                     (struct _kvs){NULL, 0},
	                     NULL);
}

struct _request
_ep_clients__read_by_id(const char * client_id)
{
	ASSERT(client_id != NULL);

	char * url = _strings_build(GLOBUS_AUTH_URL"/v2/api/clients/%s", client_id);

	return _request_init(REQUEST_OP_GET, 
	                     _F_STR(url),
	                     (struct _kvs){NULL, 0},
	                     (struct _kvs){NULL, 0},
	                     NULL);
}

struct _request
_ep_clients__read_by_fqdn(const char * fqdn)
{
	ASSERT(fqdn != NULL);

	struct _kv q_kv[] = {
	        _P_KV(_S_STR("fqdn"), _D_STR(fqdn)),
	    };

	return _request_init(REQUEST_OP_GET, 
	                     _S_STR(GLOBUS_AUTH_URL"/v2/api/clients"),
	                     (struct _kvs){q_kv, sizeof(q_kv)/sizeof(struct _kv)},
	                     (struct _kvs){NULL, 0},
	                     NULL);
}

struct _request
_ep_clients__update(const char * client_id, struct auth_client client)
{
	/*
	 * Mask unsupported fields.
	 */
	struct auth_client masked_client;
	memset(&masked_client, 0, sizeof(struct auth_client));
	masked_client.name             = client.name;
	masked_client.visibility       = client.visibility;
	masked_client.terms_of_service = client.terms_of_service;
	masked_client.privacy_policy   = client.privacy_policy;
	masked_client.required_idp     = client.required_idp;
	masked_client.preselect_idp    = client.preselect_idp;

	return _request_init(REQUEST_OP_PUT, 
	                     _F_STR(_strings_build(GLOBUS_AUTH_URL"/v2/api/clients/%s", client_id)),
	                     (struct _kvs){NULL, 0}, 
	                     (struct _kvs){NULL, 0}, 
	                     _auth_client_to_json_str(masked_client));
}

struct _request
_ep_clients__add_fqdn(const char * client_id, const char * fqdn)
{
	ASSERT(client_id);
	ASSERT(fqdn);

	return _request_init(REQUEST_OP_POST, 
	                     _F_STR(_strings_build(GLOBUS_AUTH_URL"/v2/api/clients/%s/%s", client_id, fqdn)),
	                     (struct _kvs){NULL, 0}, 
	                     (struct _kvs){NULL, 0}, 
	                     NULL);
}

struct _request
_ep_clients__delete(const char * client_id)
{
	ASSERT(client_id != NULL);

	char * url = _strings_build(GLOBUS_AUTH_URL"/v2/api/clients/%s", client_id);

	return _request_init(REQUEST_OP_DELETE,
	                     _F_STR(url), 
	                     (struct _kvs){NULL, 0},
	                     (struct _kvs){NULL, 0},
	                     NULL);
}

/************************************************
 *
 * /v2/api/scopes
 *
 ***********************************************/

// XXX where should this live?
// XXX this is not patchy
json_object *
_auth_scope_to_json_str(const struct auth_scope scope)
{
	json_object * j_obj = _json_object_new();

	_json_object_add(j_obj, "name",                 _json_string(scope.name));
	_json_object_add(j_obj, "description",          _json_string(scope.description));
	_json_object_add(j_obj, "scope_suffix",         _json_string(scope.scope_suffix));
	_json_object_add(j_obj, "advertised",           _json_bool(scope.advertised));
	_json_object_add(j_obj, "allows_refresh_token", _json_bool(scope.allows_refresh_token));

	json_object * j_array = _json_array_new();
	for (int i=0; scope.dependent_scopes && scope.dependent_scopes[i]; i++)
	{
		json_object * j_dep_scope = _json_object_new();
		_json_object_add(j_dep_scope, 
		                 "scope", 
		                 _json_string(scope.dependent_scopes[i]->scope));

		_json_object_add(j_dep_scope,
		                 "optional",
		                 _json_bool(scope.dependent_scopes[i]->optional));

		_json_object_add(j_dep_scope,
		                 "requires_refresh_token",
		                 _json_bool(scope.dependent_scopes[i]->requires_refresh_token));

		_json_array_add(j_array, j_dep_scope);
	}
	_json_object_add(j_obj, "dependent_scopes", j_array);

	json_object * j_envelope = _json_object_new();
	_json_object_add(j_envelope, "scope", j_obj);

	return j_envelope;
}

struct _request
_ep_scopes__create(const char * client_id, struct auth_scope scope)
{
	ASSERT(client_id);
	ASSERT(scope.name);
	ASSERT(scope.description);
	ASSERT(scope.scope_suffix);

	/*
	 * Mask unsupported fields.
	 */
	struct auth_scope masked_scope;
	memset(&masked_scope, 0, sizeof(struct auth_scope));
	masked_scope.name                 = scope.name;
	masked_scope.description          = scope.description;
	masked_scope.scope_suffix         = scope.scope_suffix;
	masked_scope.dependent_scopes     = scope.dependent_scopes;
	masked_scope.advertised           = scope.advertised;
	masked_scope.allows_refresh_token = scope.allows_refresh_token;

	return _request_init(REQUEST_OP_POST, 
	                     _F_STR(_strings_build(GLOBUS_AUTH_URL"/v2/api/clients/%s/scopes", client_id)),
	                     (struct _kvs){NULL, 0}, 
	                     (struct _kvs){NULL, 0}, 
	                     _auth_scope_to_json_str(masked_scope));
}

struct _request
_ep_scopes__read_all()
{
	return _request_init(REQUEST_OP_GET, 
	                     _S_STR(GLOBUS_AUTH_URL"/v2/api/scopes"),
	                     (struct _kvs){NULL, 0},
	                     (struct _kvs){NULL, 0},
	                     NULL);
}

struct _request
_ep_scopes__read_by_id(const char ** ids)
{
	ASSERT(ids != NULL && ids[0] != NULL);

	char * ids_list = _strings_build_list(',', ids);

	struct _kv q_kv[] = {
	        _P_KV(_S_STR("ids"), _F_STR(ids_list)),
	    };

	return _request_init(REQUEST_OP_GET, 
	                     _S_STR(GLOBUS_AUTH_URL"/v2/api/scopes"),
	                     (struct _kvs){q_kv, sizeof(q_kv)/sizeof(struct _kv)},
	                     (struct _kvs){NULL, 0},
	                     NULL);
}

struct _request
_ep_scopes__read_by_string(const char ** scope_strings)
{
	ASSERT(scope_strings != NULL && scope_strings[0] != NULL);

	char * scope_list = _strings_build_list(',', scope_strings);

	struct _kv q_kv[] = {
	        _P_KV(_S_STR("scope_strings"), _F_STR(scope_list)),
	    };

	return _request_init(REQUEST_OP_GET, 
	                     _S_STR(GLOBUS_AUTH_URL"/v2/api/scopes"),
	                     (struct _kvs){q_kv, sizeof(q_kv)/sizeof(struct _kv)},
	                     (struct _kvs){NULL, 0},
	                     NULL);
}

struct _request
_ep_scopes__update(const char      * scope_id,
                   struct auth_scope scope)
{
	ASSERT(scope.name);
	ASSERT(scope.description);
	ASSERT(scope.scope_suffix);

	/*
	 * Mask unsupported fields.
	 */
	struct auth_scope masked_scope;
	memset(&masked_scope, 0, sizeof(struct auth_scope));
	masked_scope.name                 = scope.name;
	masked_scope.description          = scope.description;
	masked_scope.scope_suffix         = scope.scope_suffix;
	masked_scope.dependent_scopes     = scope.dependent_scopes;
	masked_scope.advertised           = scope.advertised;
	masked_scope.allows_refresh_token = scope.allows_refresh_token;

	return _request_init(REQUEST_OP_POST, 
	                     _F_STR(_strings_build(GLOBUS_AUTH_URL"/v2/api/scopes/%s", scope_id)),
	                     (struct _kvs){NULL, 0}, 
	                     (struct _kvs){NULL, 0}, 
	                     NULL);
}

struct _request
_ep_scopes__delete(const char * scope_id)
{
	ASSERT(scope_id != NULL);

	char * url = _strings_build(GLOBUS_AUTH_URL"/v2/api/scopes/%s", scope_id);

	return _request_init(REQUEST_OP_DELETE, 
	                     _F_STR(url),
	                     (struct _kvs){NULL, 0},
	                     (struct _kvs){NULL, 0},
	                     NULL);
}
