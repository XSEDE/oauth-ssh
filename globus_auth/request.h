#ifndef _GLOBUS_AUTH_REQUEST_H_
#define _GLOBUS_AUTH_REQUEST_H_

#include "kv.h"
#include "strings.h"
#include "json.h"

typedef enum {REQUEST_OP_POST, 
              REQUEST_OP_GET, 
              REQUEST_OP_PUT, 
              REQUEST_OP_DELETE, 
              REQUEST_OP_MAX} _request_op_t;

struct _request {
	_request_op_t  op;
	struct _string url;
	struct _kvs    q_kvs;

	enum {B_TYPE_KVS, B_TYPE_JSON} b_type;
	union {
		struct _kvs   kvs;
		json_object * json;
	} b;
};

struct _request
_request_init(_request_op_t  op,
              struct _string url,
              struct _kvs    q_kvs,
              struct _kvs    b_kvs,
              json_object *  b_json); // object or string? How can we sanitize?
                                      // b_json could probably be merged with b_kvs

void
_request_free(struct _request);

char *
_request_build_url(struct _request, int sanitize);

char *
_request_build_body(struct _request, int sanitize);

#endif /* _GLOBUS_AUTH_REQUEST_H_ */
