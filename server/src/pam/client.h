#ifndef _CLIENT_H_
#define _CLIENT_H_

/*
 * System includes.
 */
#include <stdbool.h>

/*
 * Local includes.
 */
#include "json.h"

struct client {
	char ** scopes;
	char ** redirect_uris;
	char *  name;

	struct links {
		char * privacy_policy;
		char * terms_and_conditions;
	} links;

	char ** grant_types;
	char ** fqdns;
	char *  visibility;
	char *  project;
	char *  required_idp;
	char *  preselect_idp;
	char *  id;
	bool    public_client;
	char *  parent_client;
};

struct client * client_init(json_t *);
void client_fini(struct client *);

#endif /* _CLIENT_H_ */
