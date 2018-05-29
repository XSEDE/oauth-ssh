#
# Copyright The University of Chicago
# All Rights Reserved.
#

TESTS = $(check_PROGRAMS)

check_PROGRAMS   = test_utilities test_config test_module test_scope test_acct_map
test_utilities_SOURCES = test_utilities.c unit_test.c ../utilities.c ../constants.c
test_config_SOURCES = test_config.c unit_test.c ../config.c
test_scope_SOURCES  = test_scope.c unit_test.c ../scope.c
test_module_SOURCES = test_module.c unit_test.c
test_acct_map_SOURCES = test_acct_map.c unit_test.c ../acct_map.c ../utilities.c ../constants.c

AM_CFLAGS = -DUNIT_TEST $(CMOCKA_CFLAGS)

test_acct_map_CFLAGS = $(AM_CFLAGS) -I../../globus_auth/

LDADD=$(CMOCKA_LIBS)

test_module_LDADD = -ldl $(LDADD)
