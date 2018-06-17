#ifndef _INTROSPECT_H_
#define _INTROSPECT_H_

/*
 * System includes.
 */
#include <time.h>

/*
 * Local includes.
 */
#include "credentials.h"

struct introspect {
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

int
introspect(struct credentials *, 
           const char         * token,
           struct introspect  **,
           char               ** error_msg);

void
free_introspect(struct introspect *);

#endif /* _INTROSPECT_H_ */
