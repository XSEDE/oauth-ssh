/*
 * System includes.
 */
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

/*
 * Local includes.
 */
#include "client.h"
#include "logger.h"
#include "strings.h"
#include "json.h"
#include "debug.h" // always last


typedef enum { success, failure } status_t;

static status_t
check_key(jobj_t * jobj, const char * key, json_type expected_type)
{
	if (!jobj_key_exists(jobj, key))
	{
		logger(LOG_TYPE_ERROR,
		       "Client record is missing required key '%s'", key);
		return failure;
	}

	if (jobj_get_type(jobj, key) != expected_type)
	{
		logger(LOG_TYPE_ERROR,
		       "Client record has wrong type for required key '%s'", key);
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
		if (tmp) *(char **)value = strdup(tmp);
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
get_string_array(jarr_t * jarr, const char * key, void * value)
{
	char ** list = calloc(jarr_get_length(jarr)+1, sizeof(char *));
	*(char ***)value = list;

	for (int k = 0; k < jarr_get_length(jarr); k++)
	{
		if (jarr_get_type(jarr, k) != json_type_string)
		{
			logger(LOG_TYPE_ERROR, "'%s' is malformed", key);
			return failure;
		}
		list[k] = strdup(jarr_get_string(jarr, k));
	}
	return success;
}

status_t
get_scopes(jarr_t * jarr, void * value)
{
	return get_string_array(jarr, "scopes", value);
}

status_t
get_redirect_uris(jarr_t * jarr, void * value)
{
	return get_string_array(jarr, "redirect_uris", value);
}

status_t
get_grant_types(jarr_t * jarr, void * value)
{
	return get_string_array(jarr, "grant_types", value);
}

status_t
get_fqdns(jarr_t * jarr, void * value)
{
	return get_string_array(jarr, "fqdns", value);
}

status_t
get_links(jobj_t * jobj, void * value)
{
    struct links * l = value;

	if (GET_VALUE(jobj, l, privacy_policy,       string) == failure ||
        GET_VALUE(jobj, l, terms_and_conditions, string) == failure)
	{
		return failure;
	}
	return success;
}


struct client *
client_init(jobj_t * jobj)
{
	// client info is in the 'client' envelope

	const char * key = "client";
	if (check_key(jobj, key, json_type_object))
		return NULL;
	jobj = jobj_get_value(jobj, key);

	struct client * c = calloc(1, sizeof(*c));

	if (GET_CONTAINER(jobj, c, scopes,        array)  == failure ||
	    GET_CONTAINER(jobj, c, redirect_uris, array)  == failure ||
	    GET_CONTAINER(jobj, c, grant_types,   array)  == failure ||
	    GET_CONTAINER(jobj, c, fqdns,         array)  == failure ||
// XXX Skip optional fields for now
//	    GET_CONTAINER(jobj, c, links,         object) == failure ||
	    GET_VALUE(jobj, c, name,          string)  == failure ||
	    GET_VALUE(jobj, c, visibility,    string)  == failure ||
	    GET_VALUE(jobj, c, project,       string)  == failure ||
//	    GET_VALUE(jobj, c, required_idp,  string)  == failure ||
//	    GET_VALUE(jobj, c, preselect_idp, string)  == failure ||
//	    GET_VALUE(jobj, c, parent_client, string)  == failure ||
	    GET_VALUE(jobj, c, id,            string)  == failure ||
	    GET_VALUE(jobj, c, public_client, boolean) == failure)
	{
		client_fini(c);
		return NULL;
	}

	return c;
}

void
client_fini(struct client * c)
{
	if (c)
	{
		free_array(c->scopes);
		free_array(c->redirect_uris);
		free_array(c->grant_types);
		free_array(c->fqdns);
		free(c->links.privacy_policy);
		free(c->links.terms_and_conditions);
		free(c->name);
		free(c->visibility);
		free(c->project);
		free(c->required_idp);
		free(c->preselect_idp);
		free(c->id);
		free(c->parent_client);
	}
	free(c);
}

