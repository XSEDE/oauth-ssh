/*
 * System includes.
 */
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

/*
 * Local includes.
 */
#include "strings.h"
#include "config.h"
#include "parser.h"
#include "logger.h"
#include "debug.h" // always last

typedef enum { success, failure } status_t;

status_t
validate_single(const char * key, char ** values, bool already_set)
{
    if (already_set == true)
    {
        logger(LOG_TYPE_ERROR,
               "Multiple occurrences of '%s' in %s",
               key,
               CONFIG_DEFAULT_FILE);
        return failure;
    }

    if (values == NULL || values[0] == NULL)
    {
        logger(LOG_TYPE_ERROR,
               "Missing value for '%s' in %s",
               key,
               CONFIG_DEFAULT_FILE);
        return failure;
    }

    if (values[1] != NULL)
    {
        logger(LOG_TYPE_ERROR,
               "Too many values for '%s' in %s",
               key,
               CONFIG_DEFAULT_FILE);
        return failure;
    }

    return success;
}

char **
merge_values(char ** current_values, char ** new_values)
{
    int cnt = 0;
    for (int i = 0; current_values && current_values[i]; i++) cnt++;
    for (int i = 0; new_values && new_values[i]; i++) cnt++;

    char ** return_values = NULL;

    if (cnt > 0)
    {
        int j = 0;
        return_values = calloc(cnt+1, sizeof(char*));
        for (int i = 0; current_values && current_values[i]; i++)
        {
            return_values[j++] = strdup(current_values[i]);
        }
        for (int i = 0; new_values && new_values[i]; i++)
        {
            return_values[j++] = strdup(new_values[i]);
        }
    }

    return return_values;
}

static status_t
check_is_set(const char * key, bool is_set)
{
    if (!is_set)
    {
        logger(LOG_TYPE_ERROR,
               "Directive '%s' is missing from %s",
               key,
               CONFIG_DEFAULT_FILE);
        return failure;
    }
    return success;
}

static status_t
parse_file(struct config * config)
{
	FILE * fptr = fopen(CONFIG_DEFAULT_FILE, "r");
	if (!fptr)
	{
		logger(LOG_TYPE_ERROR, "Could not open %s: %m", CONFIG_DEFAULT_FILE);
		return failure;
	}

    char * key;
    char ** values;

    bool client_secret_set = false;
    bool idp_suffix_set = false;
    bool client_id_set = false;
    bool timeout_set = false;

    status_t status = failure;
    while (read_next_pair(fptr, &key, &values))
    {
        if (strcmp(key, "client_id") == 0)
        {
            status = validate_single(key, values, client_id_set);
            if (status != success)
                goto cleanup;

            config->client_id = strdup(values[0]);
            client_id_set = true;
        }
        else
        if (strcmp(key, "client_secret") == 0)
        {
            status = validate_single(key, values, client_secret_set);
            if (status != success)
                goto cleanup;

            config->client_secret = strdup(values[0]);
            client_secret_set = true;
        }
        else
        if (strcmp(key, "idp_suffix") == 0)
        {
            status = validate_single(key, values, idp_suffix_set);
            if (status != success)
                goto cleanup;

            config->idp_suffix = strdup(values[0]);
            idp_suffix_set = true;
        }
        else
        if (strcmp(key, "map_file") == 0)
        {
            char ** save_ptr = config->map_files;
            config->map_files = merge_values(config->map_files, values);
            free_array(save_ptr);
        }
        else
       if (strcmp(key, "permitted_idps") == 0)
        {
            char ** save_ptr = config->permitted_idps;
            config->permitted_idps = merge_values(config->permitted_idps,
                                                  values);
            free_array(save_ptr);
        }
        else
        if (strcmp(key, "authentication_timeout") == 0)
        {
            status = validate_single(key, values, timeout_set);
            if (status != success)
                goto cleanup;

            config->authentication_timeout = atol(values[0]);
            timeout_set = true;
        }
        else
	if (strcmp(key, "issuers") == 0)
	{
	      
	    char ** save_ptr = config->issuers;  
            config->issuers = merge_values(config->issuers,
                                                  values);
	    free_array(save_ptr);
	}
        else
	if (strcmp(key, "access_token") == 0)
	{
	      
	    char ** save_ptr = config->access_token;  
            config->access_token = merge_values(config->access_token,
                                                  values);
	    free_array(save_ptr);
	}
        else
        {
            logger(LOG_TYPE_ERROR,
                   "Unknown directive '%s' in %s",
                   key,
                   CONFIG_DEFAULT_FILE);
            status = failure;
            goto cleanup;

        }
    }

    if ((status = check_is_set("client_id", client_id_set)) == failure)
        goto cleanup;
    if ((status = check_is_set("client_secret", client_secret_set)) == failure)
        goto cleanup;

    if (!config->idp_suffix && !config->map_files)
    {
        logger(LOG_TYPE_ERROR,
            "At least one of 'idp_suffix' or 'map_file' must be defined in  %s",
             CONFIG_DEFAULT_FILE);
        status = failure;
        goto cleanup;
    }

    status = success;
cleanup:
	fclose(fptr);
    return status;
}

static status_t
parse_args(struct config * c, int flags, int argc, const char ** argv)
{
	for (int i = 0; i < argc; i++)
	{
		if (strcmp("debug", argv[i]) == 0)
			c->debug = true;
		else if (strncmp("environment=", argv[i], 12) == 0)
		{
			if (!c->environment) // skip duplicates
				c->environment = strdup(argv[i]+12);
		}
	}
	return success;
}

struct config *
config_init(int flags, int argc, const char ** argv)
{
	struct config * config = calloc(1, sizeof(*config));

	if (parse_file(config) == failure)
		goto cleanup;

	if (parse_args(config, flags, argc, argv) == failure)
		goto cleanup;

	return config;

cleanup:
	config_fini(config);
	return NULL;
}

void
config_fini(struct config * config)
{
	if (config)
	{
		free(config->client_id);
		free(config->client_secret);
		free(config->idp_suffix);
		free_array(config->map_files);
		free_array(config->permitted_idps);
		free(config->environment);
	}
	free(config);
}
