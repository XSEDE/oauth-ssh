#ifndef UNIT_TEST_H
#define UNIT_TEST_H

#ifdef UNIT_TESTING
 #include <stdarg.h>
 #include <stddef.h>
 #include <setjmp.h>
 #include <cmocka.h>
 #define static
#endif /* UNIT_TESTING */

#ifdef DEBUG
 #include <assert.h>
 #define ASSERT(x) assert(x)
#else
 #define ASSERT(x)
#endif

#endif /* UNIT_TEST_H */
