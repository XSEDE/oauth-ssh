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
 * /v2/oauth2/authorize
 *
 ***********************************************/

struct _request
_ep_authorize(const char      *  client_id, 
              const char      *  code_challenge, // Optional, native app only
              const char      *  redirect_uri,
              const char      ** scopes,
              const char      *  state,
              auth_access_type_t access_type);

/************************************************
 *
 * /v2/oauth2/token
 *
 ***********************************************/

struct _request
_ep_token__code(const char * client_id,
                const char * code_verifier, // Optional, native app only
                const char * redirect_uri,
                const char * authorization_code);
 
struct _request
_ep_token__client(const char ** scopes);

struct _request
_ep_token__dependent(const char       * access_token,
                     auth_access_type_t access_type);

struct _request
_ep_token__refresh(const char * native_client_id,  // Native App Only
                   const char * refresh_token);

struct _request
_ep_token__revoke(const char * native_client_id, // Optional, native app only
                  const char * token);

struct _request
_ep_token__validate(const char * native_client_id, // Optional, native app only
                    const char * token);

struct _request
_ep_token__introspect(const char * token, int include_id_set);

/************************************************
 *
 * /v2/oauth2/userinfo
 *
 ***********************************************/

struct _request _ep_userinfo();

/************************************************
 *
 * /v2/api/identities
 *
 ***********************************************/

struct _request
_ep_identities__by_id(const char ** ids);

struct _request
_ep_identities__by_name(const char ** usernames);

/************************************************
 *
 * /v2/api/clients
 *
 ***********************************************/

struct _request
_ep_clients__create(struct auth_client client);

struct _request
_ep_clients__read_all();

struct _request
_ep_clients__read_by_id(const char * client_id);

struct _request
_ep_clients__read_by_fqdn(const char * fqdn);

struct _request
_ep_clients__update(const char * client_id, struct auth_client client);

struct _request
_ep_clients__add_fqdn(const char * client_id, const char * fqdn);

struct _request
_ep_clients__delete(const char * client_id);

/************************************************
 *
 * /v2/api/scopes
 *
 ***********************************************/

struct _request
_ep_scopes__create(const char * client_id, struct auth_scope scope);

struct _request
_ep_scopes__read_all();

struct _request
_ep_scopes__read_by_id(const char ** ids);

struct _request
_ep_scopes__read_by_string(const char ** scope_strings);

struct _request
_ep_scopes__update(const char      * scope_id,
                   struct auth_scope scope);

struct _request
_ep_scopes__delete(const char * scope_id);

#endif /* _GLOBUS_AUTH_ENDPOINTS_H_ */
