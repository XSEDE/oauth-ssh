#include <stdlib.h>
#include <string.h>
#include "response.h"
#include "strings.h"

#include "test/unit_test.h"

void
_response_free(struct _response r)
{
	if (r.body) free(r.body);
}
