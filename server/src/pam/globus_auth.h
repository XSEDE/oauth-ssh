#ifndef _GLOBUS_AUTH_H_
#define _GLOBUS_AUTH_H_

/*
 * Local includes.
 */
#include "introspect.h"
#include "identities.h"
#include "client.h"
#include "config.h"

struct client *
get_client_resource(const struct config *);

struct introspect *
get_introspect_resource(const struct config *, const char * token);

struct identities *
get_identities_resource(const struct config *, const struct introspect *);

#endif /* _GLOBUS_AUTH_H_ */
