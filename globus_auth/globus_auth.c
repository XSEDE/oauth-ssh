#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <regex.h>
#include <time.h>

#include "auth_error.h"
#include "curl_wrapper.h"
#include "endpoint_errors.h"
#include "auth_introspect.h"
#include "endpoints.h"
#include "encode.h"
#include "error.h"
#include "json.h"

#include "tests/unit_test.h"

/************************************************
 *
 * Toggle debug mode 
 *
 ***********************************************/

void
auth_set_debug(int debug) // print to stderr if debug
{
	_curl_set_verbose(debug != 0);
}

/************************************************
 *
 * Free error struct returned by most API calls
 *
 ***********************************************/

void
auth_free_error(struct auth_error * ae)
{
	// Just pass it through
	if (ae) _auth_error_free(ae);
}


/************************************************
 *
 * /v2/oauth2/authorize
 *
 ***********************************************/

char *
auth_code_verifier(int length)
{
	char valid_chars[] = "abcdefghijklmnopqrstuvwxyz" 
	                     "ABCDEFGHIJKLMNOPQRSTUVWXYZ" 
	                     "0123456789" "-._~";

	char * code_verifier = calloc(length+1, sizeof(char));
	srandom(time(NULL));

	for (int i = 0; i < length; i++)
	{
		code_verifier[i] = valid_chars[random() % (sizeof(valid_chars)-1)];
	}
	return code_verifier;
}

char *
auth_code_grant_url(const char      *  client_id,
                    const char      *  code_verifier, // Optional, native app only
                    const char      *  redirect_uri,
                    const char      ** scopes,
                    const char      *  state,
                    auth_access_type_t access_type) /* online, offline */
{
        ASSERT(client_id    != NULL);
        ASSERT(redirect_uri != NULL);
        ASSERT(scopes       != NULL);
        ASSERT(state        != NULL);
	ASSERT(access_type >= 0 && access_type < AUTH_ACCESS_TYPE_MAX);

	char * code_challenge = _encode_code_verifier(code_verifier);
	struct _request ep_request = _ep_authorize(client_id,
	                                           code_challenge,
	                                           redirect_uri,
	                                           scopes,
	                                           state,
	                                           access_type);
	free(code_challenge);

	char * code_grant_url = _request_build_url(ep_request, 0);
	_request_free(ep_request);
	return code_grant_url;
}

/************************************************
 *
 * /v2/oauth2/token
 *
 ***********************************************/

// XXX move these helpers out to their own file
static struct auth_token *
_auth_token_from_json_obj(json_object * j_obj)
{
	struct auth_token * t = calloc(sizeof(struct auth_token), 1);

	char * scope_string = _json_to_string(j_obj, "scope");
	t->scopes           = _strings_split_to_array(scope_string); 

	t->access_token     = _json_to_string(j_obj, "access_token");
	t->resource_server  = _json_to_string(j_obj, "resource_server");
	t->expires_in       = _json_to_int(j_obj,    "expires_in");
	t->token_type       = _json_to_string(j_obj, "token_type");
	t->refresh_token    = _json_to_string(j_obj, "refresh_token");
	t->id_token         = _json_to_string(j_obj, "id_token");
	t->state            = _json_to_string(j_obj, "state");

	if (scope_string) free(scope_string);
	return t;
}

static struct auth_token **
_auth_tokens_from_json_obj(json_object * j_obj)
{
	json_object * j_other_tokens = _json_object_get(j_obj, "other_tokens");
	int cnt = _json_array_get_len(j_other_tokens);

	struct auth_token ** t = calloc(sizeof(struct auth_token *), cnt+2);

	t[0] = _auth_token_from_json_obj(j_obj);

	for (int i = 0; i < cnt; i++)
	{
		t[i+1] = _auth_token_from_json_obj(_json_array_get_idx(j_other_tokens, i));
	}

//XXX	_json_free(j_other_tokens);

	return t;
}

static struct auth_token **
_auth_tokens_from_json_str(const char * j_string)
{
	struct json_tokener *  j_tokener = json_tokener_new();
	struct json_object  *  j_obj     = json_tokener_parse_ex(j_tokener, j_string, strlen(j_string));
	struct auth_token   ** t         = _auth_tokens_from_json_obj(j_obj);

	json_tokener_free(j_tokener);
	_json_free(j_obj);
	return t;
}

void
auth_free_tokens(struct auth_token ** t)
{
	if (t)
	{
		for (int i = 0; t[i]; i++)
		{
			if (t[i]->access_token)    free(t[i]->access_token);
			if (t[i]->resource_server) free(t[i]->resource_server);
			if (t[i]->token_type)      free(t[i]->token_type);
			if (t[i]->refresh_token)   free(t[i]->refresh_token);
			if (t[i]->id_token)        free(t[i]->id_token);
			if (t[i]->state)           free(t[i]->state);

			free(t[i]);
		}
		free(t);
	}
}

#ifdef NOT
int
auth_code_grant(const char        *   client_id,
                const char        *   client_secret, // Non native app only
                const char        *   code_verifier, // Native app only
                const char        *   authorization_code,
                const char        *   redirect_uri,
                struct auth_token *** tokens)
{
	ASSERT(client_id    != NULL);
	ASSERT(redirect_uri != NULL);
	ASSERT(!client_secret ^ !code_verifier);

	struct _request ep_request = _ep_token__code(client_id,
	                                             code_verifier,
	                                             redirect_uri,
	                                             authorization_code);

	struct _response c_response = _curl_send_request(client_id, client_secret, ep_request);
	_request_free(ep_request);
	*tokens = _auth_tokens_from_json_str(c_response.body);
	_response_free(c_response);
	return 0;
}

int
auth_client_grant(const char        *   client_id,
                  const char        *   client_secret,
                  const char        **  scopes,
                  struct auth_token *** tokens)
{
	ASSERT(client_id);
	ASSERT(client_secret);
	ASSERT(scopes && scopes[0]);

	struct _request ep_request = _ep_token__client(scopes);
	struct _response c_response = _curl_send_request(client_id, client_secret, ep_request);
	_request_free(ep_request);
	*tokens = _auth_tokens_from_json_str(c_response.body);
	_response_free(c_response);

	return 0;
}

int
auth_dependent_grant(const char        *   client_id,
                     const char        *   client_secret,
                     const char        *   access_token,
                     auth_access_type_t    access_type,
                     struct auth_token *** tokens)
{
	ASSERT(client_id);
	ASSERT(client_secret);
	ASSERT(access_token);

	struct _request ep_request  = _ep_token__dependent(access_token, access_type);
	struct _response c_response = _curl_send_request(client_id, client_secret, ep_request);
	_request_free(ep_request);
	*tokens = _auth_tokens_from_json_str(c_response.body);
	_response_free(c_response);

	return 0;
}
#endif

struct auth_error *
auth_refresh_token(const char        *   client_id,
                   const char        *   client_secret, // NULL for native apps
                   const char        *   refresh_token,
                   struct auth_token *** tokens)
{
	ASSERT(client_id);
	ASSERT(refresh_token);

	// Native App support
	const char * native_app_client_id = client_id;
	if (client_secret) native_app_client_id = NULL;

	struct auth_error * ae = NULL;

	struct _request ep_request  = _ep_token__refresh(native_app_client_id, refresh_token);
	struct _response c_response;
	struct _curl_error * ce = _curl_send_request(client_id, 
	                                             client_secret, 
	                                             ep_request, 
	                                             &c_response);

	if (ce)
		ae = _auth_error_from_curl(ce);
	else if (!(ae = _auth_error_from_ep(_ep_errors_from_json_string(c_response.body))))
		*tokens = _auth_tokens_from_json_str(c_response.body);

	_request_free(ep_request);
	_response_free(c_response);

	return ae;
}

#ifdef NOT

int
auth_revoke_token(const char * client_id,
                  const char * client_secret, // NULL for native apps
                  const char * token)
{
	ASSERT(client_id);
	ASSERT(token);

	const char * native_app_client_id = client_id;
	if (!client_secret) native_app_client_id = NULL;

	struct _request ep_request = _ep_token__revoke(native_app_client_id, token);
	struct _response c_response = _curl_send_request(client_id, client_secret, ep_request);
	_request_free(ep_request);
	_response_free(c_response);

	return 0;
}

int
auth_validate_token(const char * client_id,
                    const char * client_secret, // NULL for native apps
                    const char * token)
{
	ASSERT(client_id);
	ASSERT(token);

	const char * native_app_client_id = client_id;
	if (!client_secret) native_app_client_id = NULL;

	struct _request ep_request = _ep_token__validate(native_app_client_id, token);
	struct _response c_response = _curl_send_request(client_id, client_secret, ep_request);
	_request_free(ep_request);
	_response_free(c_response);

	return 0;
}
#endif

struct auth_error *
auth_introspect_token(const char             *  client_id,
                      const char             *  client_secret, // NULL for native apps
                      const char             *  token,
                      int                       include_id_set,
                      struct auth_introspect ** introspect)
{
	ASSERT(client_id);
	ASSERT(token);

	struct auth_error * ae = NULL;

	struct _request ep_request  = _ep_token__introspect(token, include_id_set);

	struct _response c_response;
	struct _curl_error * ce = _curl_send_request(client_id, 
	                                             client_secret, 
	                                             ep_request, 
	                                             &c_response);

	if (ce)
		ae = _auth_error_from_curl(ce);
	else if (!(ae = _auth_error_from_ep(_ep_errors_from_json_string(c_response.body))))
		*introspect = _auth_introspect_from_json_str(c_response.body);

	_request_free(ep_request);
	_response_free(c_response);

	return ae;
}

void
auth_free_introspect(struct auth_introspect * i)
{
	// Pass through
	_auth_introspect_free(i);
}
#ifdef NOT

/************************************************
 *
 * /v2/oauth2/userinfo
 *
 ***********************************************/

static struct auth_userinfo *
_auth_userinfo_from_json_str(const char * j_string)
{
	struct json_tokener * j_tokener = json_tokener_new();
	struct json_object  * j_obj = json_tokener_parse_ex(j_tokener, j_string, strlen(j_string));

	struct auth_userinfo * u = calloc(sizeof(struct auth_userinfo), 1);

	u->sub                = _json_to_string(j_obj, "sub");
	u->preferred_username = _json_to_string(j_obj, "preferred_username");
	u->name               = _json_to_string(j_obj, "name");
	u->email              = _json_to_string(j_obj, "email");

	json_tokener_free(j_tokener);
	return u;
}

int
auth_get_userinfo(const char           *  bearer_token,
                  struct auth_userinfo ** userinfo)
{
	ASSERT(bearer_token);

	struct _request ep_request  = _ep_userinfo();
	struct _response c_response = _curl_send_request(NULL, bearer_token, ep_request);
	*userinfo = _auth_userinfo_from_json_str(c_response.body);
	_request_free(ep_request);
	_response_free(c_response);
}

void
auth_free_userinfo(struct auth_userinfo * u)
{
	if (u)
	{
		if (u->sub) free(u->sub);
		if (u->preferred_username) free(u->preferred_username);
		if (u->name) free(u->name);
		if (u->email) free(u->email);
		free(u);
	}
}

/************************************************
 *
 * /v2/api/identities
 *
 ***********************************************/
#endif
static int
_is_uuid(const char * string)
{
	if (strlen(string) != 36)
		return 0;

	regex_t req;
	regcomp(&req, "[0-9a-f]{8}-[0-9a-f]{4}-4[0-9]{3}-[89ab][0-9a-f]{3}-[0-9a-f]{12}", REG_ICASE);
	regmatch_t match;
	regexec(&req, string, 1, &match, 0);
	regfree(&req);

	return (match.rm_so != -1);
}

static struct auth_identity *
_auth_id_from_json_obj(json_object * j_obj)
{
	struct auth_identity * i = calloc(sizeof(struct auth_identity), 1);

	i->id           = _json_to_string(j_obj, "id");
	i->username     = _json_to_string(j_obj, "username");
	i->email        = _json_to_string(j_obj, "email");
	i->name         = _json_to_string(j_obj, "name");
	i->organization = _json_to_string(j_obj, "organization");

	char * status   = _json_to_string(j_obj, "status");
	if (strcmp(status, "unused") == 0)
		i->status = ID_STATUS_UNUSED;
	else if (strcmp(status, "used") == 0)
		i->status = ID_STATUS_USED;
	else if (strcmp(status, "private") == 0)
		i->status = ID_STATUS_PRIVATE;
	else if (strcmp(status, "closed") == 0)
		i->status = ID_STATUS_CLOSED;
#ifdef DEBUG
	else ASSERT(0);
#endif

	if (status) free(status);
	return i;
}

static struct auth_identity **
_auth_ids_from_json_obj(json_object * j_obj)
{
	json_object * j_ids = _json_object_get(j_obj, "identities");
	int cnt = json_object_array_length(j_ids);

	struct auth_identity ** i = calloc(sizeof(struct auth_identity*), cnt+1);

	for (int j = 0; j < cnt; j++)
	{
		i[j] = _auth_id_from_json_obj(_json_array_get_idx(j_ids, j));
	}

//XXX	_json_free(j_ids);

	return i;
}

static struct auth_identity **
_auth_ids_from_json_str(const char * j_string)
{
	struct json_tokener  *  j_tokener = json_tokener_new();
	struct json_object   *  j_obj     = json_tokener_parse_ex(j_tokener, j_string, strlen(j_string));
	struct auth_identity ** i         = _auth_ids_from_json_obj(j_obj);

	json_tokener_free(j_tokener);
	_json_free(j_obj);
	return i;
}

struct auth_error *
auth_get_identities(const char           *   client_id,
                    const char           *   secret_or_bearer, // Optional?
                    const char           **  ids, // NULL terminated list of UUIDs or usernames
                    struct auth_identity *** auth_ids)
{
	ASSERT(client_id);
	ASSERT(ids && ids[0]);

#ifdef DEBUG
	int _is_id = _is_uuid(ids[0]);
	for (int i = 1; ids[i]; i++)
	{
		ASSERT(_is_uuid(ids[i]) == _is_id);
	}
#endif /* DEBUG */

	struct _request ep_request;
	if(_is_uuid(ids[0]))
		ep_request = _ep_identities__by_id(ids);
	else
		ep_request = _ep_identities__by_name(ids);

	struct _response c_response;
	struct _curl_error * ce = _curl_send_request(client_id, secret_or_bearer, ep_request, &c_response);

	struct auth_error * ae = NULL;
	if (ce)
		ae = _auth_error_from_curl(ce);
	else if (!(ae = _auth_error_from_ep(_ep_errors_from_json_string(c_response.body))))
		*auth_ids = _auth_ids_from_json_str(c_response.body);

	_request_free(ep_request);
	_response_free(c_response);

	return ae;
}

void
auth_free_identities(struct auth_identity ** ids)
{
	if (ids)
	{
		for (int i = 0; ids[i]; i++)
		{
			if (ids[i]->id) free(ids[i]->id);
			if (ids[i]->username) free(ids[i]->username);
			if (ids[i]->email) free(ids[i]->email);
			if (ids[i]->name) free(ids[i]->name);
			if (ids[i]->organization) free(ids[i]->organization);
			free(ids[i]);
		}
		free(ids);
	}
}

/************************************************
 *
 * /v2/api/clients
 *
 ***********************************************/

static struct auth_client *
_auth_client_from_json_obj(json_object * j_obj)
{
	struct auth_client * c = calloc(sizeof(struct auth_client), 1);

	c->public_client    = _json_to_bool(j_obj,   "public_client");
	c->name             = _json_to_string(j_obj, "name");
	c->privacy_policy   = _json_to_string(j_obj, "privacy_policy");
	c->required_idp     = _json_to_string(j_obj, "required_idp");
	c->preselect_idp    = _json_to_string(j_obj, "preselect_idp");
	c->id               = _json_to_string(j_obj, "id");
	c->parent_id        = _json_to_string(j_obj, "parent_id");
	c->project_id       = _json_to_string(j_obj, "project_id");
	c->terms_of_service = _json_to_string(j_obj, "terms_of_service");
	c->grant_types      = _json_to_string_array(j_obj, "grant_types");
	c->scopes           = _json_to_string_array(j_obj, "scopes");
	c->fqdns            = _json_to_string_array(j_obj, "fqdns");
	c->redirect_uris    = _json_to_string_array(j_obj, "redirect_uris");

	char * value = _json_to_string(j_obj, "visibility");
	if (value)
	{
		if (strcmp(value, "public") == 0)
			c->visibility = CLIENT_VIS_PUBLIC;
		else if (strcmp(value, "private") == 0)
			c->visibility = CLIENT_VIS_PRIVATE;
#ifdef DEBUG
		else ASSERT(0);
#endif
	}

	return c;
}

static struct auth_client **
_auth_clients_from_json_obj(json_object * j_obj)
{
	json_object * j_client = _json_object_get(j_obj, "clients");

	if (!j_client) return NULL;

	int cnt = json_object_array_length(j_client);

	struct auth_client ** c = calloc(sizeof(struct auth_client*), cnt+1);

	for (int i = 0; i < cnt; i++)
	{
		c[i] = _auth_client_from_json_obj(_json_array_get_idx(j_client, i));
	}

//XXX	_json_free(j_client);

	return c;
}

static struct auth_client **
_auth_clients_from_json_str(const char * j_string)
{
	struct json_tokener *  j_tokener = json_tokener_new();
	struct json_object  *  j_obj     = json_tokener_parse_ex(j_tokener, j_string, strlen(j_string));
	struct auth_client  ** c         = _auth_clients_from_json_obj(j_obj);

	json_tokener_free(j_tokener);
	_json_free(j_obj);
	return c;
}

static struct auth_client *
_auth_client_from_json_str(const char * j_string)
{
	struct json_tokener * j_tokener = json_tokener_new();
	struct json_object  * j_env     = json_tokener_parse_ex(j_tokener, j_string, strlen(j_string));
	json_object         * j_client  = _json_object_get(j_env, "client");
	struct auth_client  * c         = _auth_client_from_json_obj(j_client);

	json_tokener_free(j_tokener);
	_json_free(j_env);
	return c;
}
#ifdef NOT

int
auth_create_client(const char         *  client_id,
                   const char         *  client_secret,
                   struct auth_client    client_definition,
                   struct auth_client ** client_created)
{
	ASSERT(client_definition.name);

	struct _request  ep_request = _ep_clients__create(client_definition);
	struct _response c_response = _curl_send_request(client_id, client_secret, ep_request);
	_request_free(ep_request);

	*client_created = _auth_client_from_json_str(c_response.body);

	_response_free(c_response);
	return 0;
}
#endif
struct auth_error *
auth_get_client(const char         *   client_id,
                const char         *   client_secret,
                const char         *   id_or_fqdn, // NULL returns all clients
                struct auth_client *** clients)
{
	ASSERT(client_id);

	struct auth_error * ae = NULL;

	struct _request ep_request;
	if (!id_or_fqdn)
		ep_request = _ep_clients__read_all();
	else if (_is_uuid(id_or_fqdn))
		ep_request = _ep_clients__read_by_id(id_or_fqdn);
	else
		ep_request = _ep_clients__read_by_fqdn(id_or_fqdn);

	struct _response c_response;
	struct _curl_error * ce = _curl_send_request(client_id, 
	                                             client_secret, 
	                                             ep_request, 
	                                             &c_response);

// XXX The JSON response may be 'client', 'clients', 'error', etc
	if (ce)
		ae = _auth_error_from_curl(ce);
	else if (!(ae = _auth_error_from_ep(_ep_errors_from_json_string(c_response.body))))
		*clients = _auth_clients_from_json_str(c_response.body);

	_request_free(ep_request);
	_response_free(c_response);

	return ae;
}
#ifdef NOT

// XXX currently PUTs are not very patchy
// Or are they? NULLs KVs are removed from the request
int
auth_update_client(const char       * client_id,
                   const char       * client_secret,
                   struct auth_client auth_client)
{
	ASSERT(auth_client.name);

	struct _request  ep_request = _ep_clients__update(client_id, auth_client);
	struct _response c_response = _curl_send_request(client_id, client_secret, ep_request);
// XXX what does this response look like?
	_request_free(ep_request);
	_response_free(c_response);
	return 0;
}

int
auth_add_client_fqdn(const char * client_id,
                     const char * client_secret,
                     const char * fqdn)
{

	struct _request  ep_request = _ep_clients__add_fqdn(client_id, fqdn);
	struct _response c_response = _curl_send_request(client_id, client_secret, ep_request);
	_request_free(ep_request);
// XXX build out the json response
	_response_free(c_response);
	return 0;
}

int
auth_delete_client(const char * client_id,
                   const char * client_secret,
                   const char * id_of_client_to_delete)
{
	ASSERT(client_id);
	ASSERT(client_secret);
	ASSERT(id_of_client_to_delete);
	ASSERT(_is_uuid(id_of_client_to_delete));

	struct _request ep_request = _ep_clients__delete(id_of_client_to_delete);

	struct _response c_response = _curl_send_request(client_id, client_secret, ep_request);
	_request_free(ep_request);
// XXX what does failure look like?
	_response_free(c_response);
	return 0;
}
#endif
void
auth_free_client(struct auth_client * client)
{
	if (client)
	{
			if (client->name) free(client->name);
			if (client->terms_of_service) free(client->terms_of_service);
			if (client->privacy_policy) free(client->privacy_policy);
			if (client->required_idp) free(client->required_idp);
			if (client->preselect_idp) free(client->preselect_idp);
			if (client->id) free(client->id);
			if (client->parent_id) free(client->parent_id);
			if (client->project_id) free(client->project_id);

			if (client->redirect_uris)
			{
				for (int j = 0; client->redirect_uris[j]; j++)
				{
					free(client->redirect_uris[j]);
				}
				free(client->redirect_uris);
			}

			if (client->grant_types)
			{
				for (int j = 0; client->grant_types[j]; j++)
				{
					free(client->grant_types[j]);
				}
				free(client->grant_types);
			}

			if (client->scopes)
			{
				for (int j = 0; client->scopes[j]; j++)
				{
					free(client->scopes[j]);
				}
				free(client->scopes);
			}

			if (client->fqdns)
			{
				for (int j = 0; client->fqdns[j]; j++)
				{
					free(client->fqdns[j]);
				}
				free(client->fqdns);
			}

			free(client);
	}
}

void
auth_free_clients(struct auth_client ** clients)
{
	if (clients)
	{
		for (int i = 0; clients[i]; i++)
		{
			auth_free_client(clients[i]);
		}
		free(clients);
	}
}

/************************************************
 *
 * /v2/api/scopes
 *
 ***********************************************/

static struct auth_scope *
_auth_scope_from_json_obj(json_object * j_obj)
{
	struct auth_scope * s = calloc(sizeof(struct auth_scope), 1);

	s->scope_suffix = _json_to_string(j_obj, "scope_suffix");
	s->name         = _json_to_string(j_obj, "name");
	s->description  = _json_to_string(j_obj, "description");
	s->id           = _json_to_string(j_obj, "id");
	s->client       = _json_to_string(j_obj, "client");
	s->advertised   = _json_to_bool(j_obj,   "advertised");
	s->allows_refresh_token = _json_to_bool(j_obj, "allows_refresh_token");

	json_object * j_dep_scopes = _json_object_get(j_obj, "dependent_scope");
	if (j_dep_scopes)
	{
		int cnt = json_object_array_length(j_dep_scopes);

		s->dependent_scopes = calloc(sizeof(struct dependent_scope *), cnt+1);
		for (int i = 0; i < cnt; i++)
		{
			s->dependent_scopes[i] = calloc(sizeof(struct dependent_scope), 1);

			json_object * j_dep_scope = _json_array_get_idx(j_dep_scopes, i);

			s->dependent_scopes[i]->scope = _json_to_string(j_dep_scope, "scope");
			s->dependent_scopes[i]->optional = _json_to_bool(j_dep_scope, "optional");
			s->dependent_scopes[i]->requires_refresh_token = _json_to_bool(j_dep_scope, "requires_refresh_token");

			_json_free(j_dep_scope);
		}

//XXX		_json_free(j_dep_scopes);
	}

	return s;
}

static struct auth_scope **
_auth_scopes_from_json_obj(json_object * j_obj)
{
	json_object * j_scope = _json_object_get(j_obj, "scopes");
	int cnt = json_object_array_length(j_scope);

	struct auth_scope ** s = calloc(sizeof(struct auth_scope*), cnt+1);

	for (int i = 0; i < cnt; i++)
	{
		s[i] = _auth_scope_from_json_obj(_json_array_get_idx(j_scope, i));
	}

//XXX	_json_free(j_scope);

	return s;
}

// XXX can we generalize all of these json->struct calls?
// XXX consolidate all auth_scope functions into scope.c
static struct auth_scope **
_auth_scopes_from_json_str(const char * j_string)
{
	struct json_tokener *  j_tokener = json_tokener_new();
	struct json_object  *  j_obj     = json_tokener_parse_ex(j_tokener, j_string, strlen(j_string));
	struct auth_scope   ** s         = _auth_scopes_from_json_obj(j_obj);

	json_tokener_free(j_tokener);
	_json_free(j_obj);
	return s;
}

static struct auth_scope *
_auth_scope_from_json_str(const char * j_string)
{
	struct json_tokener * j_tokener = json_tokener_new();
	struct json_object  * j_env     = json_tokener_parse_ex(j_tokener, j_string, strlen(j_string));
	json_object         * j_scope   = _json_object_get(j_env, "scopes");
	struct auth_scope   * s         = _auth_scope_from_json_obj(j_scope);

	json_tokener_free(j_tokener);
	_json_free(j_env);
	return s;
}

struct auth_error *
auth_create_scope(const char        *   client_id,
                  const char        *   client_secret,
                  const char        *   target_client_id,
                  struct auth_scope     scope_definition,
                  struct auth_scope *** scopes_created)
{
	ASSERT(client_id);
	ASSERT(client_secret);
// I should probably just check these in endpoints.c XXX
	ASSERT(target_client_id);
	ASSERT(scope_definition.scope_suffix);
	ASSERT(scope_definition.name);
	ASSERT(scope_definition.description);

	struct auth_error * ae = NULL;
	struct _request ep_request  = _ep_scopes__create(target_client_id, scope_definition);
	struct _response c_response;
	struct _curl_error * ce = _curl_send_request(client_id, client_secret, ep_request, &c_response);

	if (ce)
		ae = _auth_error_from_curl(ce);
	else if (!(ae = _auth_error_from_ep(_ep_errors_from_json_string(c_response.body))))
		*scopes_created = _auth_scopes_from_json_str(c_response.body);

	_request_free(ep_request);
	_response_free(c_response);
	return ae;
}

struct auth_error *
auth_get_scopes(const char        *   client_id,
                const char        *   client_secret,
                const char        **  ids_or_strings, // NULL returns all scopes
                struct auth_scope *** scopes)
{
	ASSERT(client_id);
	ASSERT(client_secret);
	ASSERT(!ids_or_strings || ids_or_strings[0]);

#ifdef DEBUG
	if (ids_or_strings)
	{
		int _is_id = _is_uuid(ids_or_strings[0]);
		for (int i = 1; ids_or_strings[i]; i++)
		{
			ASSERT(_is_uuid(ids_or_strings[i]) == _is_id);
		}
	}
#endif /* DEBUG */

	struct _request ep_request;
	if (!ids_or_strings)
		ep_request = _ep_scopes__read_all();
	else if (_is_uuid(ids_or_strings[0]))
		ep_request = _ep_scopes__read_by_id(ids_or_strings);
	else
		ep_request = _ep_scopes__read_by_string(ids_or_strings);

	struct _response c_response;
	struct _curl_error * ce = _curl_send_request(client_id, client_secret, ep_request, &c_response);

	struct auth_error * ae = NULL;
	if (ce)
		ae = _auth_error_from_curl(ce);
	else if (!(ae = _auth_error_from_ep(_ep_errors_from_json_string(c_response.body))))
		*scopes = _auth_scopes_from_json_str(c_response.body);

	_request_free(ep_request);
	_response_free(c_response);
	return ae;
}
#ifdef NOT

int
auth_update_scope(const char      * client_id,
                  const char      * client_secret,
                  const char      * scope_id,
                  struct auth_scope scope)
{
	ASSERT(client_id);
	ASSERT(client_secret);

	struct _request ep_request  = _ep_scopes__update(scope_id, scope);
	struct _response c_response = _curl_send_request(client_id, client_secret, ep_request);
	_request_free(ep_request);
// XXX parse response
	_response_free(c_response);
	return 0;
}

int
auth_delete_scope(const char * client_id,
                  const char * client_secret,
                  const char * scope_id)
{
	ASSERT(client_id);
	ASSERT(client_secret);
	ASSERT(scope_id);

	struct _request ep_request = _ep_scopes__delete(scope_id);

	struct _response c_response = _curl_send_request(client_id, client_secret, ep_request);
	_request_free(ep_request);
	_response_free(c_response);

	return 0;
}
#endif
void
auth_free_scope(struct auth_scope ** s)
{
	if (s)
	{
		for (int i = 0; s[i]; i++)
		{
			if (s[i]->scope_suffix) free(s[i]->scope_suffix);
			if (s[i]->name)         free(s[i]->name);
			if (s[i]->description) free(s[i]->description);
			if (s[i]->id)           free(s[i]->id);
			if (s[i]->client)       free(s[i]->client);
 
			if (s[i]->dependent_scopes)
			{
				for (int j = 0; s[i]->dependent_scopes[j]; j++)
				{
					if (s[i]->dependent_scopes[j]->scope)
						free (s[i]->dependent_scopes[j]->scope);
					free (s[i]->dependent_scopes[j]);
				}
				free(s[i]->dependent_scopes);
			}

			free(s[i]);
		}
		free(s);
	}
}
