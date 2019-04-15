#ifndef _JSON_H_
#define _JSON_H_

/*
 * System includes.
 */
#include<stdbool.h>
#include<json-c/json.h>

/*
 * json_t, jobj_t, jarr_t are equivalent and interchangeable. They each exist
 * to provide context at the call site for readability. jobj_t should refer
 * to values known to be type object. jarr_t should refer to values known to
 * be type array. json_t is used when the type cannot be known (ex. for return
 * values) or when a function applies to objects and arrays.
 */
typedef struct json_object json_t;
typedef json_t jobj_t;
typedef json_t jarr_t;

/*
 * Primitives.
 */

// constructor. error_msg is optional.
jobj_t * jobj_init(const char * jstring, char ** error_msg);
// destructor. call this only on json_init() returned values.
void     jobj_fini(jobj_t *);

// Call this on the key prior to any 'get' routines.
bool jobj_key_exists(jobj_t *, const char * key);

/*
 * All returned values memory allocations are tied to the parent json_t
 * returned from json_init(). Do not attempt to deallocate them individually.
 * These values are undefined after the call to json_fini on the parent json_t.
 */

// returned values are undefined if the key does not exist
json_type    jobj_get_type(jobj_t *,   const char * key);
json_t *     jobj_get_value(jobj_t *,  const char * key);
int          jobj_get_int(jobj_t *,    const char * key);
bool         jobj_get_bool(jobj_t *,   const char * key);
const char * jobj_get_string(jobj_t *, const char * key);

// returned values are undefined if the index does not exist
int          jarr_get_length(jarr_t *);
json_type    jarr_get_type(jarr_t *,    int index);
json_t *     jarr_get_index(jarr_t *,   int index);
int          jarr_get_int(jarr_t *,     int index);
bool         jarr_get_bool(jarr_t *,    int index);
const char * jarr_get_string(jarr_t *,  int index);

/*
 * JSON validation.
 */

#define FIELD &(struct field)
#define SUBFIELDS (struct field *[])

struct field {
	const char   *  key;
	bool            required;
	json_type       jtype;
	struct field ** subfields;
};

bool
is_json_object_valid(json_t * json_object, struct field ** fields);

#endif /* _JSON_H_ */
