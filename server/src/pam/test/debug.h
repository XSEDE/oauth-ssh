#ifndef _DEBUG_H__
#define _DEBUG_H__

/*
 * System includes.
 */
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>

#ifdef DEBUG
/*
 * Defining UNIT_TESTING enables memory checking in cmocka.h. This only works
 * for us if DEBUG is set so that our PAM module will also perform the memory
 * checks.
 */
#define UNIT_TESTING
#endif /* DEBUG */
#include <cmocka.h>

/*
 * CMOCKA doesn't provide these memory hooks, so we implemented them.
 */
#ifdef DEBUG
#define strdup(s)    _test_strdup(s,__FILE__,__LINE__)
#define strndup(s,n) _test_strndup(s,n,__FILE__,__LINE__)

char *
_test_strdup(const char * str, const char * file, int lineno);

char *
_test_strndup(const char *s, size_t n, const char * file, int lineno);

#endif /* DEBUG */

#endif /* _DEBUG_H__ */
