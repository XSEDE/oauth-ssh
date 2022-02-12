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

/*
 * Parse token introspect results. See introspect.h for details on which members
 * are required and which are optional.
 */

typedef enum { success, failure } status_t;

enum { required = true, optional = false};

typedef status_t (*func_t)(jobj_t * jobj, void * member_ptr);

#define GET_CONTAINER(j, i, key, type, required) \
     get_value(j, #key, json_type_##type, &i->key, get_##key, required)

#define GET_VALUE(j, i, key, type, required) \
     get_value(j, #key, json_type_##type, &i->key, NULL, required)

static status_t
get_value(jobj_t     * jobj,
          const char * key,
          json_type    jtype,
          void       * member_ptr,
          func_t       func,
          bool         required)
{
	// Check if the key exists in the JSON object
	if (!jobj_key_exists(jobj, key))
	{
		// It doesn't exist but we don't require it anyways
		if (!required)
			return success;

		// It is required, so the parsing has failed
		logger(LOG_TYPE_ERROR, "Introspect record is missing required key '%s'", key);
		return failure;
	}

	// Check the element's type
	if (jobj_get_type(jobj, key) != jtype)
	{
		// Special case were optional keys have value 'null'
		if (!required && jobj_get_type(jobj, key) == json_type_null)
			return success;

		logger(LOG_TYPE_ERROR, "Introspect record has wrong type for required key '%s'", key);
		return failure;
	}

	switch (jtype)
	{
	case json_type_int:
		*(int *)member_ptr = jobj_get_int(jobj, key);
		break;
	case json_type_string:
	{
		const char * tmp = jobj_get_string(jobj, key);
		if (tmp)
			*(char **)member_ptr = strdup(tmp);
		break;
	}
	case json_type_boolean:
		*(bool *)member_ptr = jobj_get_bool(jobj, key);
		break;

	case json_type_array:
	case json_type_object:
		return func(jobj_get_value(jobj, key), member_ptr);

	case json_type_double:
	case json_type_null:
		ASSERT(0);
		return failure;
	}
	return success;
}

status_t
get_aud(jarr_t * jarr, void * member_ptr)
{
	char ** ptr = calloc(jarr_get_length(jarr)+1, sizeof(char*));
	*((char ***)member_ptr) = ptr;

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
get_amr(jarr_t * jarr, void * member_ptr)
{
	struct amr * amr = (struct amr *)member_ptr;

	for (int k = 0; k < jarr_get_length(jarr); k++)
	{
		if (jarr_get_type(jarr, k) != json_type_string)
		{
			logger(LOG_TYPE_ERROR,
			       "Introspect field 'identities_set' is malformed");
			return failure;
		}
		const char * claim = jarr_get_string(jarr, k);
		if (strcmp(claim, "mfa") == 0)
			amr->mfa = true;
	}
	return success;
}

status_t
get_identities_set(jarr_t * jarr, void * member_ptr)
{
	char ** ptr = calloc(jarr_get_length(jarr)+1, sizeof(char*));
	*((char ***)member_ptr) = ptr;

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
get_authentications(jobj_t * jobj, void * member_ptr)
{
	struct authentication ** a = calloc(1, sizeof(*a));

	int cnt = 0;
	json_object_object_foreach(jobj, k, v)
	{
		a = realloc(a, sizeof(*a) * (cnt+2));
		a[cnt] = calloc(1, sizeof(**a));

		a[cnt]->identity_id = strdup(k);
		if (GET_VALUE(v, a[cnt], idp,       string, required) == failure ||
		    GET_VALUE(v, a[cnt], auth_time, int,    required) == failure)
		{
			return failure;
		}

		// TODO: Not positive if this is required or optional
		if (GET_CONTAINER(v, a[cnt], amr, array, optional)  == failure)
			return failure;

		a[++cnt] = NULL;
	}
	*(struct authentication ***)member_ptr = a;
	return success;
}

status_t
get_session_info(jobj_t * jobj, void * member_ptr)
{
	struct session_info * s = calloc(1, sizeof(*s));
	*(struct session_info **) member_ptr = s;

	if (GET_VALUE(jobj, s, session_id, string, required) == failure)
		return failure;

	// TODO: not sure if this is required or optional
	return GET_CONTAINER(jobj, s, authentications, object, optional);
}

struct introspect *
introspect_init(jobj_t * jobj)
{
	struct introspect * i = calloc(1, sizeof(struct introspect));
	status_t status;

	if ((status = GET_VALUE(jobj, i, active, boolean, required)) == failure)
		goto cleanup;

	if (i->active == false)
		goto cleanup;

	if ((status = GET_VALUE(jobj, i, scope,     string, required)) == failure ||
	    (status = GET_VALUE(jobj, i, client_id, string, required)) == failure ||
	    (status = GET_VALUE(jobj, i, sub,       string, required)) == failure ||
	    (status = GET_VALUE(jobj, i, username,  string, required)) == failure ||
	    (status = GET_VALUE(jobj, i, iss,       string, required)) == failure ||
	    (status = GET_VALUE(jobj, i, email,     string, required)) == failure ||
	    (status = GET_VALUE(jobj, i, exp,       int,    required)) == failure ||
	    (status = GET_VALUE(jobj, i, iat,       int,    required)) == failure ||
	    (status = GET_VALUE(jobj, i, nbf,       int,    required)) == failure)
	{
		goto cleanup;
	}

	if ((status = GET_CONTAINER(jobj, i, aud,            array,  required)) == failure ||
	    (status = GET_CONTAINER(jobj, i, identities_set, array,  optional)) == failure ||
	    (status = GET_CONTAINER(jobj, i, session_info,   object, optional)) == failure)
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
		free(i);
	}
}
