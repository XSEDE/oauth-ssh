/*
 * System includes.
 */
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/*
 * Local includes.
 */
#include "acct_map_module.h"
#include "acct_map.h"
#include "acct_map_module_file.h"
#include "acct_map_module_idp.h"

struct acct_map {
	void * idp;
	void * file;
};

struct acct_map *
acct_map_init()
{
	return calloc(1, sizeof(struct acct_map));
}

void
acct_map_free(struct acct_map * acct_map)
{
	if (acct_map->file)
		acct_map_module_map_file.finalize(acct_map->file);
	if (acct_map->idp)
		acct_map_module_idp_suffix.finalize(acct_map->idp);
	free(acct_map);
}

int
acct_map_add_module(struct acct_map * acct_map,
                    const char      * module,
                    const char      * option)
{
	/*
	 * Built ins
	 */
	if (strcmp(module, AcctMapModuleMapFile) == 0)
	{
		return acct_map_module_map_file.initialize(&acct_map->file, option);
	}

	if (strcmp(module, AcctMapModuleIdpSuffix) == 0)
	{
		return acct_map_module_idp_suffix.initialize(&acct_map->idp, option);
	}

	return -ENOENT;
}

static void
add_account(char * account, char *** accounts)
{
	if (!account) return;

	int index = 0;
	for (index = 0; *accounts && accounts[index]; index++)
	{
		if (strcmp(account, (*accounts)[index]) == 0)
		{
			free(account);
			return;
		}
	}

	*accounts = realloc(*accounts, (index+2) * sizeof(char *));
	(*accounts)[index] = account;
	(*accounts)[index+1] = NULL;
}

char **
acct_map_lookup(struct acct_map * acct_map, char * usernames_or_ids[])
{
	char ** accounts = NULL;

	for (int i = 0; usernames_or_ids && usernames_or_ids[i]; i++)
	{
		char * account;
		char * id = usernames_or_ids[i];

		if (acct_map->file)
		{
			account = acct_map_module_map_file.lookup(acct_map->file, id);
			add_account(account, &accounts);
		}

		if (acct_map->idp)
		{
			account = acct_map_module_idp_suffix.lookup(acct_map->idp, id);
			add_account(account, &accounts);
		}
	}

	return accounts;
}
