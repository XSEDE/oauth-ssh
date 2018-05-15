#
# Copyright The University of Chicago
# All Rights Reserved.
#

TESTS = test_account_mapping

check_PROGRAMS = test_account_mapping
test_account_mapping_SOURCES = test_account_mapping.c
test_account_mapping_LDADD   = ../libaccount_mapping.la $(CMOCKA_LIBS)

AM_CFLAGS = -DUNIT_TEST
AM_CPPFLAGS = -I.. $(CMOCKA_CFLAGS)

