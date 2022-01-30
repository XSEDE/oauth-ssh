#ifndef _DEBUG_H__
#define _DEBUG_H__

/*
 * System includes.
 */
#include <assert.h>
#include <stdlib.h>

// Disable assert in release code
#define ASSERT(x)
// Hide functions in release code
#define STATIC static
// Prevent weak symbols in release code
#define TESTS_CAN_OVERRIDE

#ifdef DEBUG

// Allow assert in debug code
#undef ASSERT
#define ASSERT(x) assert(x)
// Unhide functions in debug code
#undef STATIC
// Allow weak symbols in debug code
#undef TESTS_CAN_OVERRIDE
#define TESTS_CAN_OVERRIDE __attribute__((weak))

#ifndef NO_MEMORY_MACROS
/*
 * Avoid these macros if the caller doesn't want them expanded.
 */
#define malloc(size)      _test_malloc(size,__FILE__,__LINE__)
#define realloc(ptr,size) _test_realloc(ptr,size,__FILE__,__LINE__)
#define calloc(num,size)  _test_calloc(num,size,__FILE__,__LINE__)
#define strdup(str)       _test_strdup(str,__FILE__,__LINE__)
#define strndup(str,size) _test_strndup(str,size,__FILE__,__LINE__)
#define free(ptr)         _test_free(ptr,__FILE__,__LINE__)

#endif

TESTS_CAN_OVERRIDE
void *
_test_malloc(size_t size, const char * file, int lineno);

TESTS_CAN_OVERRIDE
void *
_test_realloc(void * ptr, size_t size, const char * file, int lineno);

TESTS_CAN_OVERRIDE
void *
_test_calloc(size_t nmemb, size_t size, const char * file, int lineno);

TESTS_CAN_OVERRIDE
char *
_test_strdup(const char * str, const char * file, int lineno);

TESTS_CAN_OVERRIDE
char *
_test_strndup(const char * str, size_t size, const char * file, int lineno);

TESTS_CAN_OVERRIDE
void
_test_free(void * ptr, const char * file, int lineno);

#endif /* DEBUG */

#endif /* _DEBUG_H__ */
