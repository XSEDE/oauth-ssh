/*
 * System includes.
 */
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

/*
 * Local includes.
 */
#include "identities.h"
#include "logger.h"
#include "json.h"
#include "debug.h" // always last

typedef enum { success, failure } status_t;

static status_t
check_key(jobj_t * jobj, const char * key, json_type jtype)
{
	if (!jobj_key_exists(jobj, key))
	{
		logger(LOG_TYPE_ERROR,
		       "Identities record is missing required key '%s'", key);
		return failure;
	}

	if (jobj_get_type(jobj, key) != jtype)
	{
		logger(LOG_TYPE_ERROR,
		       "Identities record has wrong type for required key '%s'", key);
		return failure;
	}
	return success;
}

typedef status_t (*func_t)(jobj_t * jobj, void * value);

#define GET_CONTAINER(j, i, key, type) \
     get_value(j, #key, json_type_##type, &i->key, get_##key)

#define GET_VALUE(j, i, key, type) \
     get_value(j, #key, json_type_##type, &i->key, NULL)

static status_t
get_value(jobj_t     * jobj,
          const char * key,
          json_type    jtype,
          void       * value,
          func_t       func)
{
	if (check_key(jobj, key, jtype) == failure)
		return failure;

	const char * tmp;

	switch (jtype)
	{
	case json_type_int:
		*(int *)value = jobj_get_int(jobj, key);
		break;
	case json_type_string:
		tmp = jobj_get_string(jobj, key);
		if (tmp)
			*(char **)value = strdup(tmp);
		break;
	case json_type_boolean:
		*(bool *)value = jobj_get_bool(jobj, key);
		break;

	case json_type_array:
	case json_type_object:
		return func(jobj_get_value(jobj, key), value);

	case json_type_double:
	case json_type_null:
		ASSERT(0);
		return failure;
	}
	return success;
}

status_t
get_domains(jarr_t * jarr, void * value)
{
	char ** domains = calloc(jarr_get_length(jarr)+1, sizeof(char *));
	*(char ***)value = domains;

	for (int k = 0; k < jarr_get_length(jarr); k++)
	{
		if (jarr_get_type(jarr, k) != json_type_string)
		{
			logger(LOG_TYPE_ERROR, "'domains' is malformed");
			return failure;
		}
		domains[k] = strdup(jarr_get_string(jarr, k));
	}
	return success;
}

status_t
get_alternative_names(jarr_t * jarr, void * value)
{
	char ** names = calloc(jarr_get_length(jarr)+1, sizeof(char *));
	*(char ***)value = names;

	for (int k = 0; k < jarr_get_length(jarr); k++)
	{
		if (jarr_get_type(jarr, k) != json_type_string)
		{
			logger(LOG_TYPE_ERROR, "'alternative_names' is malformed");
			return failure;
		}
		names[k] = strdup(jarr_get_string(jarr, k));
	}
	return success;
}

status_t
get_identity_providers(jarr_t * jarr, void * value)
{
	struct identity_provider ** idp =
	                     calloc(jarr_get_length(jarr)+1, sizeof(*idp));
	*(struct identity_provider ***)value = idp;

	for (int k = 0; k < jarr_get_length(jarr); k++)
	{
		if (jarr_get_type(jarr, k) != json_type_object)
		{
			logger(LOG_TYPE_ERROR, "'identity_providers' is malformed");
			return failure;
		}

		jobj_t * jobj = jarr_get_index(jarr, k);

		idp[k] = calloc(1, sizeof(struct identity_provider));

		if (GET_CONTAINER(jobj, idp[k], domains,           array) == failure  ||
		    GET_CONTAINER(jobj, idp[k], alternative_names, array) == failure  ||
		    GET_VALUE(jobj,     idp[k], id,                string) == failure ||
		    GET_VALUE(jobj,     idp[k], short_name,        string) == failure ||
		    GET_VALUE(jobj,     idp[k], name,              string) == failure)
		{
			return failure;
		}
	}
	return success;
}

status_t
get_included(jobj_t * jobj, void * value)
{
	struct included * i = value;
	return GET_CONTAINER(jobj, i, identity_providers, array);
}

status_t
get_identities(jarr_t * jarr, void * value)
{
	struct identity ** i = calloc(jarr_get_length(jarr)+1, sizeof(*i));
	*(struct identity ***)value = i;

	for (int k = 0; k < jarr_get_length(jarr); k++)
	{
		if (jarr_get_type(jarr, k) != json_type_object)
		{
			logger(LOG_TYPE_ERROR, "'identities' is malformed");
			return failure;
		}

		jobj_t * jobj = jarr_get_index(jarr, k);

		i[k] = calloc(1, sizeof(struct identity));
// XXX some IDPs can hide certain values
		if (GET_VALUE(jobj, i[k], username,          string) == failure ||
		    GET_VALUE(jobj, i[k], status,            string) == failure ||
		    GET_VALUE(jobj, i[k], id,                string) == failure ||
		    GET_VALUE(jobj, i[k], identity_provider, string) == failure)
		    //GET_VALUE(jobj, i[k], name,              string) == failure ||
		    //GET_VALUE(jobj, i[k], organization,      string) == failure ||
		    //GET_VALUE(jobj, i[k], email,             string) == failure)
		{
			return failure;
		}
	}
	return success;
}

struct identities *
identities_init(jobj_t * jobj)
{
	struct identities * i = calloc(1, sizeof(*i));

	if (GET_CONTAINER(jobj, i, included,   object) == failure ||
	    GET_CONTAINER(jobj, i, identities, array)  == failure)
	{
		identities_fini(i);
		return NULL;
	}
	return i;
}

void
identities_fini(struct identities * i)
{
	if (i)
	{
		if (i->included.identity_providers)
		{
			for (int j = 0; i->included.identity_providers[j]; j++)
			{
				free(i->included.identity_providers[j]->id);
				free(i->included.identity_providers[j]->name);
				free(i->included.identity_providers[j]);
			}
		}
		free(i->included.identity_providers);

		if (i->identities)
		{
			for (int j = 0; i->identities[j]; j++)
			{
				free(i->identities[j]->username);
				free(i->identities[j]->status);
				//free(i->identities[j]->name);
				free(i->identities[j]->id);
				free(i->identities[j]->identity_provider);
				//free(i->identities[j]->organization);
				//free(i->identities[j]->email);
				free(i->identities[j]);
			}
		}
		free(i->identities);
	}
	free(i);
}
