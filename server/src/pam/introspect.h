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

/*
 * See https://docs.globus.org/api/auth/reference/#token-introspect
 * Requried fields are guaranteed to be present if the introspect_init()
 * succeeds. Optional fields can either be missing from the JSON response
 * or set to 'null'; these are treated as equivalent. Optional fields will
 * have value 0/NULL if not present; care must be taken when accessing those
 * members. Ignored fields are not parsed, even if they are present.
 *
 *
 * There's a special exception to the Optional/Required settings; if active
 * is false, all other fields are 0/NULL.
 */

struct introspect {
	bool    active;                     // Required
	char *  scope;                      // Required
	char *  client_id;                  // Required
	char *  sub;                        // Required
	char *  username;                   // Required
	char ** aud;                        // Required
	char *  iss;                        // Required
	time_t  exp;                        // Required
	time_t  iat;                        // Required
	time_t  nbf;                        // Required
	char *  email;                      // Required
	char ** identities_set;             // Optional
//	struct identity_set_detail **;      // Ignored
	struct session_info {               // Optional
		char * session_id;          // Required
		struct authentication {
			char * identity_id; // Required
			char * idp;         // Required
			int    auth_time;   // Required
			struct amr {
			    bool mfa;
			} amr;              // Optional
		} ** authentications;       // Optional
	} * session_info;                   // Optional
};

struct introspect * introspect_init(json_t *);
void introspect_fini(struct introspect *);

#endif /* _INTROSPECT_H_ */
