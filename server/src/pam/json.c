/*
 * System includes.
 */
#include <json-c/json.h>
#include <string.h>

/*
 * Local includes.
 */
#include "json.h"
#include "strings.h"
#include "debug.h" // always last

jobj_t *
jobj_init(const char * jstring, char ** error_msg)
{
	if (error_msg) *error_msg = NULL;

	struct json_object * jobj = NULL;
	struct json_tokener * tokener = json_tokener_new();
	jobj = json_tokener_parse_ex(tokener, jstring, strlen(jstring));

	enum json_tokener_error jerr;
	jerr = json_tokener_get_error(tokener);
	if (jerr != json_tokener_success && error_msg)
		*error_msg = sformat("Could not convert JSON string %s: %s",
		                      jstring,
		                      json_tokener_error_desc(jerr));

	json_tokener_free(tokener);
	return jobj;
}

void
jobj_fini(jobj_t * jobj)
{
	if (jobj)
		json_object_put(jobj);
}

// Call this on the key prior to any 'get' routines.
bool
jobj_key_exists(jobj_t * jobj, const char * key)
{
	ASSERT(json_object_get_type(jobj) == json_type_object);
	json_object_object_foreach(jobj, k, v)
	{
		if (strcmp(key, k) == 0)
			return true;
	}
	return false;
}

/*
 * All returned values memory allocations are tied to the parent json_t
 * returned from json_init(). Do not attempt to deallocate them individually.
 * These values are undefined after the call to json_fini on the parent json_t.
 */

// returned values are undefined if the key does not exist
json_type
jobj_get_type(jobj_t * jobj, const char * key)
{
	ASSERT(json_object_get_type(jobj) == json_type_object);
	ASSERT(jobj_key_exists(jobj, key));
	struct json_object * jtmp;
	json_object_object_get_ex(jobj, key, &jtmp);
	return json_object_get_type(jtmp);
}

json_t *
jobj_get_value(jobj_t * jobj,  const char * key)
{
	ASSERT(json_object_get_type(jobj) == json_type_object);
	ASSERT(jobj_key_exists(jobj, key));
	ASSERT(jobj_get_type(jobj, key) == json_type_object ||
	       jobj_get_type(jobj, key) == json_type_array);
	struct json_object * jtmp;
	json_object_object_get_ex(jobj, key, &jtmp);
	return jtmp;
}

int
jobj_get_int(jobj_t * jobj, const char * key)
{
	ASSERT(json_object_get_type(jobj) == json_type_object);
	ASSERT(jobj_key_exists(jobj, key));
	ASSERT(jobj_get_type(jobj, key) == json_type_int);

	struct json_object * jtmp;
	json_object_object_get_ex(jobj, key, &jtmp);
	return json_object_get_int(jtmp);
}

bool
jobj_get_bool(jobj_t * jobj, const char * key)
{
	ASSERT(json_object_get_type(jobj) == json_type_object);
	ASSERT(jobj_key_exists(jobj, key));
	ASSERT(jobj_get_type(jobj, key) == json_type_boolean);

	struct json_object * jtmp;
	json_object_object_get_ex(jobj, key, &jtmp);
	return json_object_get_boolean(jtmp);
}

const char *
jobj_get_string(jobj_t * jobj, const char * key)
{
	ASSERT(json_object_get_type(jobj) == json_type_object);

	struct json_object * jtmp;
	json_object_object_get_ex(jobj, key, &jtmp);
	return json_object_get_string(jtmp);
}

// returned values are undefined if the index does not exist
int
jarr_get_length(jarr_t * jarr)
{
	ASSERT(json_object_get_type(jarr) == json_type_array);
	return json_object_array_length(jarr);
}

json_type
jarr_get_type(jarr_t * jarr, int index)
{
	ASSERT(json_object_get_type(jarr) == json_type_array);
	ASSERT(jarr_get_length(jarr) > index);
    return json_object_get_type(json_object_array_get_idx(jarr, index));
}

json_t *
jarr_get_index(jarr_t * jarr, int index)
{
	ASSERT(json_object_get_type(jarr) == json_type_array);
	ASSERT(jarr_get_length(jarr) > index);

	json_t * jtmp = json_object_array_get_idx(jarr, index);
	ASSERT(json_object_get_type(jtmp) == json_type_object ||
	       json_object_get_type(jtmp) == json_type_array);
	return jtmp;
}

int
jarr_get_int(jarr_t * jarr, int index)
{
	ASSERT(json_object_get_type(jarr) == json_type_array);
	ASSERT(jarr_get_length(jarr) > index);
	ASSERT(jarr_get_type(jarr, index) == json_type_int);

	json_t * jint = json_object_array_get_idx(jarr, index);
	ASSERT(json_object_get_type(jint) == json_type_int);
	return json_object_get_int(jint);
}

bool
jarr_get_bool(jarr_t * jarr, int index)
{
	ASSERT(json_object_get_type(jarr) == json_type_array);
	ASSERT(jarr_get_length(jarr) > index);
	ASSERT(jarr_get_type(jarr, index) == json_type_boolean);

	json_t * jbool = json_object_array_get_idx(jarr, index);
	ASSERT(json_object_get_type(jbool) == json_type_boolean);
	return json_object_get_boolean(jbool);
}

const char *
jarr_get_string(jarr_t * jarr, int index)
{
	ASSERT(json_object_get_type(jarr) == json_type_array);
	ASSERT(jarr_get_length(jarr) > index);
	ASSERT(jarr_get_type(jarr, index) == json_type_string);

	json_t * jstring = json_object_array_get_idx(jarr, index);
	ASSERT(json_object_get_type(jstring) == json_type_string);
	return json_object_get_string(jstring);
}
