#
# Copyright The University of Chicago
# All Rights Reserved.
#

TESTS = test_utilities

check_PROGRAMS   = test_utilities
test_utilities_SOURCES = test_utilities.c ../utilities.c ../constants.c

AM_CFLAGS = -DUNIT_TEST $(CMOCKA_CFLAGS) -std=c99

LDADD=$(CMOCKA_LIBS)
