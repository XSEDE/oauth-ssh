#ifdef DEBUG

/*
 * System includes.
 */
#include <stdlib.h>
#include <string.h>

/*
 * Local includes.
 */
#include "debug.h" // always last

/*
 * CMOCKA doesn't provide the following hooks for memory checking.
 */

char *
_test_strdup(const char * str, const char * file, int lineno)
{
	char * s = _test_malloc(strlen(str)+1, file, lineno);
	return strcpy(s, str);
}

char *
_test_strndup(const char * str, size_t n, const char * file, int lineno)
{
	char * s = _test_calloc(n+1, sizeof(char), file, lineno);
	return strncpy(s, str, n);
}

#endif /* DEBUG */
