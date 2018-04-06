#include "json.h"
#include "strings.h"
#include "tests/unit_test.h"

// XXX in debug mode, I'd like this to assert() if an expected field is not found.
// XXX and assert() when an unexpected field is found
// That will require some input from a higher tier
json_object *
_json_bool(int true)
{
	return json_object_new_boolean(true != 0);
}

json_object *
_json_string(const char * value)
{
	if (!value) return NULL;

	return json_object_new_string(value);
}

json_object *
_json_object_new()
{
	return json_object_new_object();
}

void
_json_free(json_object * j_obj)
{
	if (!j_obj) return;
	json_object_put(j_obj);
}

void
_json_object_add(json_object * j_obj, const char * key, json_object * value)
{
	ASSERT(key);

	if (!value) return;
	if (!j_obj)
	{
		_json_free(value);
		return;
	}

	json_object_object_add(j_obj, key, value);
}

/*
 * XXX This does not increment the reference count, should it?
 */
json_object *
_json_object_get(json_object * j_obj, const char * key)
{
	json_object * j_value = NULL;
	json_object_object_get_ex(j_obj, key, &j_value);
	return j_value;

}

json_object *
_json_array_new()
{
	return json_object_new_array();
}

void
_json_array_add(json_object * array, json_object * value)
{
	if (!value) return;

	if (!array)
	{
		_json_free(value);
		return;
	}

	json_object_array_add(array, value);
}

json_object *
_json_array_get_idx(json_object * array, int index)
{
// XXX do I need to _json_free the returned object?
	return json_object_array_get_idx(array, index);
}

int
_json_array_get_len(json_object * array)
{
	if (!array) return 0;
	return json_object_array_length(array);
}

int
_json_to_bool(json_object * j_obj, const char * name)
{
	json_object * j_value = _json_object_get(j_obj, name);
	int                  value   = json_object_get_boolean(j_value);
//XXX	_json_free(j_value);
	return value;
}

int
_json_to_int(json_object * j_obj, const char * name)
{
	json_object * j_value = _json_object_get(j_obj, name);
	int                  value   = json_object_get_int(j_value);
//XXX	_json_free(j_value);
	return value;
}

char *
_json_to_string(json_object * j_obj, const char * name)
{
	json_object * j_value = _json_object_get(j_obj, name);
	const char         * s_value = json_object_get_string(j_value);
	char               * value   = s_value ? strdup(s_value) : NULL;
//XXX	_json_free(j_value);
	return value;
}

char **
_json_to_string_array(json_object * j_obj, const char * name)
{
	json_object * j_value = _json_object_get(j_obj, name);
	int cnt = _json_array_get_len(j_value);

	char ** string_array = NULL;
	if (cnt)
	{
		string_array = calloc(sizeof(char *), cnt+1);
		for (int i = 0; i < cnt; i++)
		{
			json_object * j_string = _json_array_get_idx(j_value, i);
			string_array[i] = strdup(json_object_get_string(j_string));
		}
	}

//XXX	_json_free(j_value);
	return string_array;
}

/*
 * Returns a string whose lifetime is that of object, no need to free() it.
 */
const char *
_json_obj_to_json_str(json_object * j_obj)
{
	return json_object_to_json_string_ext(j_obj, JSON_C_TO_STRING_SPACED | JSON_C_TO_STRING_PRETTY);
}

