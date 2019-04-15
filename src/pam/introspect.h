#ifndef _INTROSPECT_H_
#define _INTROSPECT_H_

/*
 * System includes.
 */
#include <stdbool.h>

/*
 * Local includes.
 */
#include "json.h"

struct introspect {
	bool    active;
	char *  scope;
	char *  client_id;
	char *  sub;
	char *  username;
	char ** aud;
	char *  iss;
	time_t  exp;
	time_t  iat;
	time_t  nbf;
	char *  email;
	char ** identities_set;
	struct session_info {
		char * session_id;
		struct authentication {
			char * identity_id; // user's uuid
			char * idp; // uuid of IdP
			int    auth_time;
		} ** authentications;
	} * session_info;
};

struct introspect * introspect_init(json_t *);
void introspect_fini(struct introspect *);

#endif /* _INTROSPECT_H_ */
