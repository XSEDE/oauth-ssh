#
# Copyright The University of Chicago
# All Rights Reserved.
#

TESTS = pam_test

check_PROGRAMS   = pam_test
pam_test_SOURCES = pam_test.c \
                   test_common.c \
                   ../pam.c \
                   ../../credentials.c

pam_test_LDFLAGS="-Wl,--wrap=pam_get_item"
pam_test_LDADD = ../../globus_auth/libglobus_auth.la \
                 ../../account_mapping/libaccount_mapping.la \
                 -lcurl -lssl -lcrypto -ljson-c

AM_CFLAGS = -DUNIT_TEST
AM_CPPFLAGS = -I../../globus_auth -I../../account_mapping -I../../

