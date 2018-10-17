#ifndef _UNIT_TEST_H_
#define _UNIT_TEST_H_

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <string.h>
#include <stdlib.h>
#include <cmocka.h>

#undef strdup
#define strdup test_strdup

#undef strndup
#define strndup test_strndup

char * test_strdup(const char *s);
char * test_strndup(const char *s, size_t n);

#endif /* _UNIT_TEST_H_ */

