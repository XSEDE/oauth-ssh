#ifndef _GLOBUS_AUTH_TOKEN_INTROSPECT_H_
#define _GLOBUS_AUTH_TOKEN_INTROSPECT_H_

#include "globus_auth.h" // for struct auth_introspect

// Returns:
//   - NULL if j_string is NULL
//   - struct auth_introspect if j_string is valid
struct auth_introspect *
_auth_introspect_from_json_str(const char * j_string);

void
_auth_introspect_free(struct auth_introspect *);

#endif /* _GLOBUS_AUTH_TOKEN_INTROSPECT_H_ */
