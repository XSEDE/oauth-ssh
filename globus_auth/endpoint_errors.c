#include <string.h>
#include "endpoint_errors.h"
#include "json.h"

#include "tests/unit_test.h"

// Example Globus Auth API error:
// {"errors": [{"status": "401", "id": "601c1b08-0792-11e8-aed2-0e47636aca6c", "code": "UNAUTHORIZED", "detail": "Basic auth failed", "title": "Unauthorized"}], "error_description": "Unauthorized", "error": "unauthorized"}
//{"error":"unsupported_grant_type"}

struct _ep_errors *
_ep_errors_from_json_string(const char * j_string)
{
	if (! j_string) return NULL;

        struct json_tokener * j_tokener = json_tokener_new();
        struct json_object  * j_errors  = json_tokener_parse_ex(j_tokener, j_string, strlen(j_string));

	struct _ep_errors * epe = NULL;

        ASSERT(j_errors);

	json_object * j_err_arr = _json_object_get(j_errors, "errors");

	if (j_err_arr)
	{
		epe = calloc(sizeof(*epe), 1);

		epe->error_cnt = _json_array_get_len(j_err_arr); // XXX Use our json interface
		epe->errors    = calloc(sizeof(*(epe->errors)), epe->error_cnt);

		for (int i = 0; i < epe->error_cnt; i++)
		{
			json_object * j_entry = _json_array_get_idx(j_err_arr, i);
			epe->errors[i].status = _json_to_int(j_entry,    "status");
			epe->errors[i].id     = _json_to_string(j_entry, "id");
			epe->errors[i].code   = _json_to_string(j_entry, "code");
			epe->errors[i].detail = _json_to_string(j_entry, "detail");
			epe->errors[i].title  = _json_to_string(j_entry, "title");
		}

		epe->error_description = _json_to_string(j_errors, "error_description");
		epe->error = _json_to_string(j_errors, "error");
	} else
	{
		char * errmsg = _json_to_string(j_errors, "error");
		if (errmsg)
		{
			epe = calloc(sizeof(*epe), 1);
			epe->error_description = errmsg;
			epe->error = strdup(errmsg);
		}
	}


cleanup:
	json_tokener_free(j_tokener);
	_json_free(j_errors);

	return epe;
}

void
_ep_errors_free(struct _ep_errors * epe)
{
	if (epe)
	{
		for (int e = 0; e < epe->error_cnt; e++)
		{
			free(epe->errors[e].id);
			free(epe->errors[e].code);
			free(epe->errors[e].detail);
			free(epe->errors[e].title);
		}
		free(epe->errors);
		free(epe->error_description);
		free(epe->error);
		free(epe);
	}
}
