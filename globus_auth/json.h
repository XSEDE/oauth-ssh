#ifndef _GLOBUS_AUTH_JSON_H
#define _GLOBUS_AUTH_JSON_H

#include <json-c/json.h>
#include <globus_auth.h>

/*
 * LIMIT THIS SET OF FUNCTIONS TO PRIMITIVE TYPES. (no structs)
 */

/*
 * Create JSON objects.
 */
json_object *
_json_bool(int true);

json_object *
_json_string(const char * value);

json_object *
_json_object_new();

/*
 * Free JSON objects. (Arrays too)
 */
void
_json_free(json_object * object);

/*
 *  JSON object handling
 */
void
_json_object_add(json_object * object, const char * key, json_object * value);

json_object *
_json_object_get(json_object * object, const char * key);

/*
 * JSON arrays
 */
json_object *
_json_array_new();

void
_json_array_add(json_object * array, json_object * value);

json_object *
_json_array_get_idx(json_object * array, int index);

int
_json_array_get_len(json_object * array);

/*
 * Convert JSON objects to basic types.
 */
int
_json_to_bool(json_object * j_obj, const char * name);

int
_json_to_int(json_object * j_obj, const char * name);

char *
_json_to_string(json_object * j_obj, const char * name);

char **
_json_to_string_array(json_object * j_obj, const char * name);

/*
 * Returns a string whose lifetime is that of object, no need to free() it.
 */
const char *
_json_obj_to_json_str(json_object * object);

#endif /* _GLOBUS_AUTH_JSON_H */
