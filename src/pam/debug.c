#ifdef DEBUG

/*
 * System includes.
 */
#include <stdlib.h>
#include <string.h>

/*
 * Local includes.
 */
// Don't pollute us with memory macros
#define NO_MEMORY_MACROS
#include "debug.h"

TESTS_CAN_OVERRIDE
void *
_test_malloc(size_t size, const char * file, int lineno)
{
	return malloc(size);
}

TESTS_CAN_OVERRIDE
void *
_test_realloc(void * ptr, size_t size, const char * file, int lineno)
{
	return realloc(ptr, size);
}

TESTS_CAN_OVERRIDE
void *
_test_calloc(size_t nmemb, size_t size, const char * file, int lineno)
{
	return calloc(nmemb, size);
}

TESTS_CAN_OVERRIDE
char *
_test_strdup(const char * str, const char * file, int lineno)
{
	return strdup(str);
}

TESTS_CAN_OVERRIDE
char *
_test_strndup(const char * str, size_t size, const char * file, int lineno)
{
	return strndup(str, size);
}

TESTS_CAN_OVERRIDE
void
_test_free(void * ptr, const char * file, int lineno)
{
	return free(ptr);
}

#endif /* DEBUG */
