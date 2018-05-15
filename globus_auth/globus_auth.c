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
#include "error.h"
#include "json.h"

#include "test/unit_test.h"

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
 * /v2/oauth2/token
 *
 ***********************************************/

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

/************************************************
 *
 * /v2/api/identities
 *
 ***********************************************/
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

