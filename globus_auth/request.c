#include <string.h>

#include "request.h"

#include "tests/unit_test.h"

struct _request
_request_init(_request_op_t op,
              struct _string url,
              struct _kvs    q_kvs,
              struct _kvs    b_kvs,
              json_object *  b_json)
{
	ASSERT(op >= 0 && op < REQUEST_OP_MAX);
	ASSERT(url.str != NULL);

	ASSERT((!b_json + !q_kvs.cnt + !b_kvs.cnt) >= 2);

	struct _request request;

	request.op     = op;
	request.url    = url;
	request.q_kvs  = _kvs_copy(q_kvs);

	request.b.kvs  = _kvs_copy(b_kvs);
	request.b_type = B_TYPE_KVS;
	if (b_json)
	{
		request.b_type = B_TYPE_JSON;
		request.b.json = b_json;
	}

	return request;
}

void
_request_free(struct _request r)
{
	_kvs_free(r.q_kvs);
	if (r.b_type == B_TYPE_KVS)
		_kvs_free(r.b.kvs);
	else
		_json_free(r.b.json);
}

char *
_request_build_url(struct _request r, int sanitize)
{
	return _strings_build_url(r.url.str, r.q_kvs, sanitize);
}

char *
_request_build_body(struct _request r, int sanitize)
{
	if (r.b_type == B_TYPE_KVS)
		return _strings_build_kv_list(r.b.kvs, '&', 0, sanitize);
	return strdup(_json_obj_to_json_str(r.b.json));
}
