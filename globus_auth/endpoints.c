#include <stdlib.h>
#include <string.h>

#include "endpoints.h"
#include "request.h"
#include "strings.h"
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
 * /v2/oauth2/token
 *
 ***********************************************/

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

