/*
 * System includes.
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/*
 * Local includes.
 */
#include "acct_map_module_file.h"
#include "config.h"

const char * AcctMapModuleMapFile = "map_file";

struct module_handle {
	struct config * config;
};

static int
initialize(void ** handle, const char * option)
{
	struct config * config = config_init();

	int retval = config_load(config, option);
	if (retval)
	{
		config_free(config);
		return retval;
	}

	*handle = calloc(1, sizeof(struct module_handle));
	((struct module_handle *)*handle)->config = config;
	return 0;
}

static char *
lookup(void * handle, const char * id)
{
	struct module_handle * module_handle = handle;

	char * account;
	config_get_value(module_handle->config, id, &account);
	return account;
}

static void
finalize(void * handle)
{
	config_free(((struct module_handle *)handle)->config);
	free(handle);
}

struct acct_map_module acct_map_module_map_file = {
	.initialize = initialize,
	.lookup     = lookup,
	.finalize   = finalize,
};
