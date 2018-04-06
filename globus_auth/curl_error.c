#include <stdlib.h>
#include <string.h>
#include "curl_error.h"
#include "tests/unit_test.h"

/**********************************************************
 *
 * Error handling routines.
 *
 *********************************************************/

struct _curl_error *
_curl_error(CURLcode code, const char * errmsg, int msglen)
{
	struct _curl_error * ce = NULL;
	if (code)
	{
		ce = calloc(sizeof(struct _curl_error), 1);
		ce->code = code;
		ASSERT(sizeof(ce->errmsg) >= msglen);
		memcpy(ce->errmsg, errmsg, sizeof(ce->errmsg));
	}

	return ce;
}

void
_curl_error_free(struct _curl_error * ce)
{
	free(ce);
}
