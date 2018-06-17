#ifndef _JSON_H_
#define _JSON_H_

/*
 * Rule of thumb for this submodule: If the function returns a
 * struct json_t *, you must json_free() it.
 */

typedef struct json_object json_t;

json_t * json_init(const char * string, char ** errmsg);
void json_free(json_t *);

/*
 * These return 'value' from {'key' : value}
 */
int          json_get_int(json_t *,    const char * key);
int          json_get_bool(json_t *,   const char * key);
const char * json_get_string(json_t *, const char * key);
json_t *     json_get_object(json_t *, const char * key);

/*
 * These return 'value' from {value}
 */
int          json_to_int(json_t *);
int          json_to_bool(json_t *);
const char * json_to_string(json_t *);


// Returns [1,2] from {"key" : [1,2]}. Use json_array_idx() on result
json_t * json_get_array(json_t *, const char * key);
int      json_array_len(json_t *);
// Returns the object at the index, you must use json_to_*() to get the value.
json_t * json_array_idx(json_t *, int index);


#endif /* _JSON_H_ */
