/*
 * System includes.
 */
#include <stdlib.h>
#include <string.h>

/*
 * Local includes.
 */
#include "acct_map_module_idp.h"

const char * AcctMapModuleIdpSuffix = "idp_suffix";

struct module_handle {
	char * idp_suffix;
};

static const char *
get_provider(const char * id_username)
{
	char * ptr = strrchr(id_username, '@');

	if (ptr) return ptr + 1;
	return NULL;
}

static int
identity_is_from_provider(const char * id_username,
                          const char * idp_suffix)
{
	const char * provider = get_provider(id_username);
	if (!provider) return 0;

	return provider && strcasecmp(provider, idp_suffix) == 0;
}

static int
initialize(void ** handle, const char * option)
{
	*handle = calloc(1, sizeof(struct module_handle));

	((struct module_handle *) *handle)->idp_suffix = strdup(option);
	return 0;
}

static char *
lookup(void * handle, const char * id)
{
	char * idp_suffix = ((struct module_handle *)handle)->idp_suffix;

	if (identity_is_from_provider(id, idp_suffix))
		return strndup(id, strrchr(id, '@') - id);

	return NULL;
}

static void
finalize(void * handle)
{
	free(((struct module_handle *)handle)->idp_suffix);
	free(handle);
}

struct acct_map_module acct_map_module_idp_suffix = {
	.initialize = initialize,
	.lookup     = lookup,
	.finalize   = finalize,
};
