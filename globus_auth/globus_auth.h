#ifndef _GLOBUS_AUTH_H_
#define _GLOBUS_AUTH_H_

#include <time.h>

/************************************************
 * 
 * Error struct returned by most API calls
 * 
 ***********************************************/
struct auth_error {
	const char         * error_message;
	struct _auth_error * _ae; // Opaque struct
};

void
auth_free_error(struct auth_error *);

char *
auth_code_verifier(int length);

typedef enum { AUTH_ACCESS_TYPE_ONLINE,
               AUTH_ACCESS_TYPE_OFFLINE,
               AUTH_ACCESS_TYPE_MAX } auth_access_type_t;

void
auth_set_debug(int debug); // print to stderr if debug


/************************************************
 * 
 * /v2/oauth2/authorize
 * 
 ***********************************************/
char *
auth_code_grant_url(const char      *  client_id,
                    const char      *  code_verifier, // Optional, native app only
                    const char      *  redirect_uri,
                    const char      ** scopes,
                    const char      *  state,
                    auth_access_type_t access_type); /* online, offline */

/************************************************
 * 
 * /v2/oauth2/token
 * 
 ***********************************************/

struct auth_token {
	char *  access_token;
	char ** scopes;
	char *  resource_server;
	time_t  expires_in;
	char *  token_type;
	char *  refresh_token;
	char *  id_token;
	char *  state;
};

void
auth_free_tokens(struct auth_token ** tokens);

int
auth_code_grant(const char        *   client_id,
                const char        *   client_secret, // Non native app only
                const char        *   code_verifier, // Native app only
                const char        *   authorization_code,
                const char        *   redirect_uri,
                struct auth_token *** tokens);

int
auth_client_grant(const char        *   client_id,
                  const char        *   client_secret,
                  const char        **  scopes,
                  struct auth_token *** tokens);

int
auth_dependent_grant(const char        *   client_id,
                     const char        *   client_secret, // XXX Native App support?
                     const char        *   access_token,
                     auth_access_type_t    access_type,
                     struct auth_token *** tokens);

struct auth_error *
auth_refresh_token(const char        *   client_id,
                   const char        *   client_secret, // NULL for native apps
                   const char        *   refresh_token,
                   struct auth_token *** tokens);

int
auth_revoke_token(const char * client_id,
                  const char * client_secret, // NULL for native apps
                  const char * token);

int
auth_validate_token(const char * client_id,
                    const char * client_secret, // NULL for native apps
                    const char * token);

struct auth_introspect {
	int     active;
	char ** scopes; // NULL terminated
	char *  sub;    // Effective identity UUID
	char *  username;
	char *  display_name;
	char *  email;
	char *  client_id;  // UUID of client token issued to
	char ** audiences;  // NULL terminated
	char *  issuer;     // https://auth.globus.org
	time_t  expiry;     // Seconds since 1970 UTC when token expires
	time_t  issued_at;  // Seconds since 1970 UTC when token was issued
	time_t  not_before; // Seconds since 1970 UTC when token was issued
	char ** identities; // NULL terminated
};

struct auth_error *
auth_introspect_token(const char * client_id,
                      const char * client_secret, // NULL for native apps
                      const char * token,
                      int          include_id_set,
                      struct auth_introspect **);

void
auth_free_introspect(struct auth_introspect *);

/************************************************
 * 
 * /v2/oauth2/userinfo
 * 
 ***********************************************/
struct auth_userinfo {
	char * sub; // UUID
	char * preferred_username;
	char * name;
	char * email;
};

int
auth_get_userinfo(const char           *  bearer_token,
                  struct auth_userinfo ** userinfo);

void
auth_free_userinfo(struct auth_userinfo*);

/************************************************
 * 
 * /v2/api/identities
 * 
 ***********************************************/
typedef enum {ID_STATUS_UNUSED, 
              ID_STATUS_USED, 
              ID_STATUS_PRIVATE, 
              ID_STATUS_CLOSED} auth_id_status_t;
struct auth_identity {
	char           * id;
	char           * username;
	auth_id_status_t status;
	char           * email;
	char           * name;
	char           * organization;
};

struct auth_error *
auth_get_identities(const char           *   client_id,
                    const char           *   secret_or_bearer, // Optional?
                    const char           **  ids, // NULL terminated list of UUIDs or usernames
                    struct auth_identity *** auth_ids);

void
auth_free_identities(struct auth_identity **);

/************************************************
 * 
 * /v2/api/clients
 * 
 ***********************************************/

/*
 * keys with NULL values are not put into request.
 * keys with zero-length values indicate unset value to Auth
 */

typedef enum {CLIENT_VIS_PUBLIC, CLIENT_VIS_PRIVATE} client_vis_t;
struct auth_client {
	/*
	 * These values can be set on client creation.
	 */
	int     public_client;   // (REQUIRED) 0 if is native app

	/*
	 * These values can be set on client creation or update.
	 */
	char      *  name;             // (REQUIRED) display name shown to users on consents
	client_vis_t visibility;       // (REQUIRED) 'public' or 'private'. default: 'private'
	char      ** redirect_uris;    // (OPTIONAL) URIs in OAuth authorization flows
	char      *  terms_of_service; // (OPTIONAL) URL 
	char      *  privacy_policy;   // (OPTIONAL) URL
	char      *  required_idp;     // (OPTIONAL) IdP UUID
	char      *  preselect_idp;    // (OPTIONAL) IdP UUID

	/*
	 * These values can not be set directly.
	 */
	char *  id;              // A unique ID generated by Globus Auth
	char *  parent_id;       // id of the client’s parent, or null if client has no parent
	char ** grant_types;     // List of OAuth grant types the client can use
	char ** scopes;          // List of scopes IDs belonging to this client
	char ** fqdns;           // List of all FQDNs the client has proven possession of
	char *  project_id;      // UUID of project containing client
};

int
auth_create_client(const char         *  client_id,
                   const char         *  client_secret,
                   struct auth_client    client_definition,
                   struct auth_client ** client_created);

struct auth_error *
auth_get_client(const char         *   client_id,
                const char         *   client_secret,
                const char         *   id_or_fqdn, // NULL returns all clients
                struct auth_client *** clients);


int
auth_update_client(const char       * client_id,
                   const char       * client_secret,
                   struct auth_client auth_client);

struct auth_fqdn {
	char * fqdn;
	struct {
		struct {
			int advertised;
			int allows_refresh_tokens;
// XXX dependent_scopes = []
			char * description;
			char * id;
			char * name;
			char * scope_string;
		} ** scopes;
	} included;
};

int
auth_add_client_fqdn(const char * client_id,
                     const char * client_secret,
                     const char * fqdn);

int
auth_delete_client(const char * client_id,
                   const char * client_secret,
                   const char * id_of_client_to_delete);

void
auth_free_client(struct auth_client * client);

void
auth_free_clients(struct auth_client ** clients);

/************************************************
 * 
 * /v2/api/scopes
 * 
 ***********************************************/
struct dependent_scope {
        char * scope; // The ID of the dependent scope
        int    optional;
        int    requires_refresh_token;
};

struct auth_scope {
	/*
	 * These values can be set on client creation.
	 */
	char * scope_suffix; // (REQUIRED)

	/*
	 * These values can be set on creation or update.
	 */
	char * name;        // (REQUIRED) on create
	char * description; // (REQUIRED) on create
	int    advertised;
	int    allows_refresh_token;
	struct dependent_scope ** dependent_scopes;

	/*
	 * These values can not be set directly.
	 */
	char * id;     // UUID of scope
	char * client; // UUID of client
};

struct auth_error *
auth_create_scope(const char        *   client_id,
                  const char        *   client_secret,
                  const char        *   target_client_id,
                  struct auth_scope     scope_definition,
                  struct auth_scope *** scopes_created);

struct auth_error *
auth_get_scopes(const char        *   client_id,
                const char        *   client_secret,
                const char        **  ids_or_strings, // NULL returns all scopes
                struct auth_scope *** scopes);

int
auth_update_scope(const char      * client_id,
                  const char      * client_secret,
                  const char      * scope_id,
                  struct auth_scope scope);

int
auth_delete_scope(const char * client_id,
                  const char * client_secret,
                  const char * scope_id);

// XXX plural?
void
auth_free_scope(struct auth_scope ** scopes);

#endif /* _GLOBUS_AUTH_H_ */
