/*
 * Copyright The University of Chicago
 * All Rights Reserved.
 */

#include <stdlib.h>
#include <string.h>
#include "account_mapping.h"

#ifdef UNIT_TEST
 #define static 
#endif /* UNIT_TEST */

struct account_mapping {
	char *  auth_identity;
	char ** local_accounts;
};

static struct account_mapping account_mappings[] = {
	// jasonalt@globus.org (Prod)
	{"06672755-9088-4d1c-9bf2-23c2d8c5d451", (char *[]){"centos", NULL}},
	// jasonalt@globus.org (Sandbox)
	{"7bb58e97-a695-4bbb-a8b5-36b709c12ab6", (char *[]){"centos", NULL}},
};

char **
acct_map_id_to_accts(const char *const* auth_identities)
{
	for (int i = 0; i < sizeof(account_mappings)/sizeof(struct account_mapping); i++)
	{
		for (int j = 0; auth_identities[j]; j++)
		{
			if (strcmp(auth_identities[j], account_mappings[i].auth_identity) == 0)
				return account_mappings[i].local_accounts;
		}
	}

	return NULL;
}

void
acct_map_free_list(char ** account_list)
{
	/* Do nothing for now */
}

