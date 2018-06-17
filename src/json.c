/*
 * System includes.
 */
#include <string.h>
#include <json-c/json.h>

/*
 * Local includes.
 */
#include "json.h"

/*
 * We are providing a thin wrapper to generalize the calls to JSON-C
 * so as to not bind us to a single implementation more than is necessary.
 */


json_t *
json_init(const char * string, char ** errmsg)
{
	struct json_tokener * tokener = json_tokener_new();

	struct json_object * jobj = json_tokener_parse_ex(tokener,
	                                                  string,
	                                                  strlen(string));

	enum json_tokener_error jerr;
	jerr = json_tokener_get_error(tokener);
	if (jerr != json_tokener_success)
	{
		jobj = NULL;
		*errmsg = strdup(json_tokener_error_desc(jerr));
	}

	json_tokener_free(tokener);

	return jobj;
}

void
json_free(json_t * json)
{
	json_object_put(json);
}

int
json_get_int(json_t * json, const char * key)
{
	struct json_object * jobj;
	json_object_object_get_ex(json, key, &jobj);
	return json_object_get_int(jobj);
}

int
json_get_bool(json_t * json, const char * key)
{
	struct json_object * jobj;
	json_object_object_get_ex(json, key, &jobj);
	return json_object_get_boolean(jobj);
}

const char *
json_get_string(json_t * json, const char * key)
{
	struct json_object * jobj;
	json_object_object_get_ex(json, key, &jobj);
	return json_object_get_string(jobj);
}

int
json_to_int(json_t * json)
{
	return json_object_get_int(json);
}

int
json_to_bool(json_t * json)
{
	return json_object_get_boolean(json);
}

const char *
json_to_string(json_t * json)
{
	return json_object_get_string(json);
}

json_t *
json_get_array(json_t * json, const char * key)
{
	return json_get_object(json, key);
}

int
json_array_len(json_t * json)
{
	return json_object_array_length(json);
}

json_t *
json_array_idx(json_t * json, int index)
{
	json_t * jobj = json_object_array_get_idx(json, index);
	json_object_get(jobj); // increment the ref count
	return jobj;
}

json_t *
json_get_object(json_t * json, const char * key)
{
	struct json_object * jobj;
	json_object_object_get_ex(json, key, &jobj);
	json_object_get(jobj);
	return jobj;
}
