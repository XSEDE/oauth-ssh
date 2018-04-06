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

void
auth_set_debug(int debug); // print to stderr if debug


/************************************************
 * 
 * /v2/oauth2/token
 * 
 ***********************************************/

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

#endif /* _GLOBUS_AUTH_H_ */
