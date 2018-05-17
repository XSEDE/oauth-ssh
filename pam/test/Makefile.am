#
# Copyright The University of Chicago
# All Rights Reserved.
#

TESTS = $(check_PROGRAMS)

check_PROGRAMS   = test_utilities test_config test_module test_scope
test_utilities_SOURCES = test_utilities.c unit_test.c ../utilities.c ../constants.c
test_config_SOURCES = test_config.c unit_test.c ../config.c
test_scope_SOURCES  = test_scope.c unit_test.c ../scope.c
test_module_SOURCES = test_module.c unit_test.c

AM_CFLAGS = -DUNIT_TEST $(CMOCKA_CFLAGS) -std=c99


LDADD=$(CMOCKA_LIBS)

test_module_LDADD = -ldl $(LDADD)
