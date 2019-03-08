#ifndef _IDENTITIES_H_
#define _IDENTITIES_H_

/*
 * Local includes.
 */
#include "json.h"

struct identities {
	struct included {
		struct identity_provider {
			char ** domains;
			char *  id;
			char ** alternative_names;
			char *  short_name;
			char *  name;
		} ** identity_providers;
	} included;

	struct identity {
		char * username;
		char * status;
//		char * name; // could be null
		char * id;
		char * identity_provider;
//		char * organization; // could be null
//		char * email; // could be null
	} ** identities;
};

struct identities * identities_init(json_t *);
void identities_fini(struct identities *);

#endif /* _IDENTITIES_H_ */
