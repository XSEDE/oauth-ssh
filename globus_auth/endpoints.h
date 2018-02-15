#ifndef _GLOBUS_AUTH_ENDPOINTS_H_
#define _GLOBUS_AUTH_ENDPOINTS_H_

#include "globus_auth.h"
#include "request.h"

/************************************************************************
 * Naming scheme: For easier auditing, function names have this syntax:
 *   _<prefix>_<endpoint_path>__<action>
 *
 * Where:
 *   leading '_' implies an internal function
 *   <prefix> identifies this file "ep" = endpoints
 *   <endpoint_path> Auth API path
 *   <action> CRUD or specific action that endpoint
 ***********************************************************************/

/************************************************
 *
 * /v2/oauth2/token
 *
 ***********************************************/

struct _request
_ep_token__introspect(const char * token, int include_id_set);

/************************************************
 *
 * /v2/api/identities
 *
 ***********************************************/

struct _request
_ep_identities__by_id(const char ** ids);

struct _request
_ep_identities__by_name(const char ** usernames);

#endif /* _GLOBUS_AUTH_ENDPOINTS_H_ */
