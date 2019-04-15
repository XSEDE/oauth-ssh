#ifndef _ACCOUNT_MAP_H_
#define _ACCOUNT_MAP_H_

/*
 * System includes.
 */
#include <stdbool.h>

/*
 * Local includes.
 */
#include "identities.h"
#include "config.h"

struct account_map {
	char *  username;
	char *  id;
	char ** accounts;

	struct account_map * next;
};

struct account_map *
account_map_init(const struct config *, const struct identities *);

void
account_map_fini(struct account_map *);

bool
is_acct_in_map(const struct account_map * map, const char * acct);

const char *
acct_to_username(const struct account_map * map, const char * acct);

#endif /* _ACCOUNT_MAP_H_ */
