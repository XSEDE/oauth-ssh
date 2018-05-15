#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <globus_auth.h>

static void
test_(void **state) {
        (void) state; /* unused */
}

int
main()
{
        const struct CMUnitTest tests[] = {
                cmocka_unit_test(test_),
        };
        return cmocka_run_group_tests(tests, NULL, NULL);
}

