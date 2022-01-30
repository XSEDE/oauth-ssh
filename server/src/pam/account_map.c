/*
 * System includes.
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/*
 * Local includes.
 */
#include "account_map.h"
#include "strings.h"
#include "logger.h"
#include "parser.h"
#include "debug.h" // always last

/*******************************************************************************
 * Internal (Private) Functions
 ******************************************************************************/

static void
_add_acct_mapping(struct account_map    ** map,
                 const struct identity *  id,
                 const char            *  acct)
{
	for (struct account_map * ptr = *map; ptr; ptr = ptr->next)
	{
		if (strcmp(ptr->id, id->id) == 0)
		{
			if (!key_in_list(CONST(char *, ptr->accounts), acct))
				insert(&ptr->accounts, acct);
			return;
		}
	}

	struct account_map * ptr = calloc(1, sizeof(*ptr));
	ptr->username = strdup(id->username);
	ptr->id = strdup(id->id);
	insert(&ptr->accounts, acct);
	ptr->next = *map;
	*map = ptr;
}

static char *
_acct_from_username(const char * username)
{
	char * ptr = strchr(username, '@');
	ASSERT(ptr);
	char * acct = calloc(ptr - username + 1, sizeof(char));
	strncpy(acct, username, ptr - username);
	return acct;
}

static bool
_id_matches_suffix(const struct identity * id, const char * suffix)
{
	char * ptr = strchr(id->username, '@');
	if (!ptr)
		return false;

	return (strcmp(ptr+1, suffix) == 0);
}

/*
 * For each line in each map_file, add the 'id' => 'acct' mapping if
 * 'acct' is in 'identities'.
 */
static struct account_map *
_build_map_file_account_map(const struct config     * config,
                            const struct identities * identities)
{
	struct account_map * map = NULL;

	for (int i = 0; config->map_files && config->map_files[i]; i++)
	{
		// We want to continue even if a map file is missing or unreadable
		// to allow users to continue to log in with the mappings available.
		FILE * fptr = fopen(config->map_files[i], "r");
		if (!fptr)
		{
			logger(LOG_TYPE_ERROR,
			       "Could not open %s: %m",
			       config->map_files[i]);
			continue;
		}

		char * key = NULL;
		char ** values = NULL;
		while (read_next_pair(fptr, &key, &values))
		{
			for (int i = 0; identities->identities[i]; i++)
			{
				struct identity * id = identities->identities[i];
				if (strcmp(id->username, key) == 0 || strcmp(id->id, key) == 0)
				{
					for (int v = 0; values && values[v]; v++)
					{
						_add_acct_mapping(&map, id, values[v]);
					}
					break;
				}
			}
			free(key);
			free_array(values);
		}
		fclose(fptr);
	}
	return map;
}

/*******************************************************************************
 * Public Functions
 ******************************************************************************/

struct account_map *
account_map_init(const struct config * config, const struct identities * ids)
{
	// Collect every matching map file entry
	struct account_map * map = _build_map_file_account_map(config, ids);

	// If the admin wants to perform implicit idp_suffix mapping
	if (config->idp_suffix)
	{
		// For each linked identity
		for (int i = 0; ids->identities && ids->identities[i]; i++)
		{
			// If the identity is from this IdP
			if (_id_matches_suffix(ids->identities[i], config->idp_suffix))
			{
				// Add the username as an allowed local account
				char * acct = _acct_from_username(ids->identities[i]->username);
				_add_acct_mapping(&map, ids->identities[i], acct);
				free(acct);
			}
		}
	}
	return map;
}

void
account_map_fini(struct account_map * account_map)
{
	struct account_map * tmp;

	while ((tmp = account_map))
	{
		free(tmp->username);
		free(tmp->id);
		free_array(tmp->accounts);
		account_map = account_map->next;
		free(tmp);
	}
}

bool
is_acct_in_map(const struct account_map * map, const char * acct)
{
	for (; map; map = map->next)
	{
		if (key_in_list(CONST(char *,map->accounts), acct))
			return true;
	}
	return false;
}

const char *
acct_to_username(const struct account_map * map, const char * acct)
{
	for (; map; map = map->next)
	{
		if (key_in_list(CONST(char *,map->accounts), acct))
			return map->username;
	}
	return NULL;
}
