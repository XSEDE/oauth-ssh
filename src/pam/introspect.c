/*
 * System includes.
 */
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

/*
 * Local includes.
 */
#include "introspect.h"
#include "strings.h"
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
		       "Introspect record is missing required key '%s'", key);
		return failure;
	}

	if (jobj_get_type(jobj, key) != jtype)
	{
		logger(LOG_TYPE_ERROR,
		       "Introspect record has wrong type for required key '%s'", key);
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
get_aud(jarr_t * jarr, void * value)
{
	char ** ptr = calloc(jarr_get_length(jarr)+1, sizeof(char*));
	*((char ***)value) = ptr;

	for (int k = 0; k < jarr_get_length(jarr); k++)
	{
		if (jarr_get_type(jarr, k) != json_type_string)
		{
			logger(LOG_TYPE_ERROR, "Introspect field 'aud' is malformed");
			return failure;
		}
		ptr[k] = strdup(jarr_get_string(jarr, k));
	}
	return success;
}

status_t
get_identities_set(jarr_t * jarr, void * value)
{
	char ** ptr = calloc(jarr_get_length(jarr)+1, sizeof(char*));
	*((char ***)value) = ptr;

	for (int k = 0; k < jarr_get_length(jarr); k++)
	{
		if (jarr_get_type(jarr, k) != json_type_string)
		{
			logger(LOG_TYPE_ERROR,
			       "Introspect field 'identities_set' is malformed");
			return failure;
		}
		ptr[k] = strdup(jarr_get_string(jarr, k));
	}
	return success;
}

status_t
get_authentications(jobj_t * jobj, void * value)
{
	struct authentication ** a = NULL;

	int cnt = 0;
	json_object_object_foreach(jobj, k, v)
	{
		a = realloc(a, sizeof(*a) * (cnt+2));
		a[cnt] = calloc(1, sizeof(**a));

		a[cnt]->identity_id = strdup(k);
		if (GET_VALUE(v, a[cnt], idp,       string) == failure ||
		    GET_VALUE(v, a[cnt], auth_time, int)    == failure)
		{
			return failure;
		}

		a[++cnt] = NULL;
	}
	*(struct authentication ***)value = a;
	return success;
}

status_t
get_session_info(jobj_t * jobj, void * value)
{
	struct session_info * s = calloc(1, sizeof(*s));
	*(struct session_info **) value = s;

	if (GET_VALUE(jobj, s, session_id, string) == failure)
		return failure;

	return GET_CONTAINER(jobj, s, authentications, object);
}

struct introspect *
introspect_init(jobj_t * jobj)
{
	struct introspect * i = calloc(1, sizeof(struct introspect));
	status_t status;

	if ((status = GET_VALUE(jobj, i, active, boolean)) == failure)
		goto cleanup;

	if (i->active == false)
		goto cleanup;

	if ((status = GET_VALUE(jobj, i, scope,     string)) == failure ||
	    (status = GET_VALUE(jobj, i, client_id, string)) == failure ||
	    (status = GET_VALUE(jobj, i, sub,       string)) == failure ||
	    (status = GET_VALUE(jobj, i, username,  string)) == failure ||
	    (status = GET_VALUE(jobj, i, iss,       string)) == failure ||
	    (status = GET_VALUE(jobj, i, email,     string)) == failure ||
	    (status = GET_VALUE(jobj, i, exp,       int))    == failure ||
	    (status = GET_VALUE(jobj, i, iat,       int))    == failure ||
	    (status = GET_VALUE(jobj, i, nbf,       int))    == failure)
	{
		goto cleanup;
	}

	if ((status = GET_CONTAINER(jobj, i, aud,            array))  == failure ||
	    (status = GET_CONTAINER(jobj, i, identities_set, array))  == failure ||
	    (status = GET_CONTAINER(jobj, i, session_info,   object)) == failure)
	{
		goto cleanup;
	}
cleanup:
	if (status == success)
		return i;

	introspect_fini(i);
	return NULL;
};

void
introspect_fini(struct introspect * i)
{
	if (i)
	{
		free(i->scope);
		free(i->client_id);
		free(i->sub);
		free(i->username);
		free(i->iss);
		free(i->email);

		free_array(i->aud);
		free_array(i->identities_set);

		if (i->session_info)
		{
			free(i->session_info->session_id);

			typeof(i->session_info->authentications) authentications;
			authentications = i->session_info->authentications;
			for (int j = 0; authentications && authentications[j]; j++)
			{
				free(authentications[j]->identity_id);
				free(authentications[j]->idp);
				free(authentications[j]);
			}
			free(i->session_info->authentications);
		}
		free(i->session_info);
	}
	free(i);
}
