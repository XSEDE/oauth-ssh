#include <strings.h>
#include <string.h>
#include <stdlib.h>

#include <globus_auth.h>
#include "acct_map.h"

const char * 
get_provider(const char * id_username)
{
	char * ptr = strrchr(id_username, '@');

	if (ptr) return ptr + 1;
	return NULL;
}

int
get_username(const char * id_username)
{
	char * ptr = strrchr(id_username, '@');

	if (!ptr) return 0;

	return ptr-id_username;
}

int
identity_is_from_provider(const char * id_username,
                          const char * idp_suffix)
{
	const char * provider = get_provider(id_username);
	if (!provider) return 0;

	return provider && strcasecmp(provider, idp_suffix) == 0;
}

char **
acct_map_idp_suffix(const char * idp_suffix,  const struct auth_identity ** auth_ids)
{
	int matching_accounts = 0;
	for (int i = 0; auth_ids && auth_ids[i]; i++)
	{
		if (identity_is_from_provider(auth_ids[i]->username, idp_suffix))
			matching_accounts++;
	}

	char ** mapped_accounts = NULL;
	if (matching_accounts > 0)
	{
		mapped_accounts = calloc(matching_accounts + 1, sizeof(char *));
		for (int i = 0, j = 0; auth_ids && auth_ids[i]; i++)
		{
			if (identity_is_from_provider(auth_ids[i]->username, idp_suffix))
			{
				int length = get_username(auth_ids[i]->username);
				mapped_accounts[j++] = strndup(auth_ids[i]->username, length);
			}
		}
	}

	return mapped_accounts;
}

void
acct_map_free_list(char ** account_list)
{
	if (account_list)
	{
		for (int i = 0; account_list[i]; i++)
			free(account_list[i]);
		free(account_list);
	}
}
