#
# Copyright The University of Chicago
# All Rights Reserved.
#

TESTS = $(check_PROGRAMS)

check_PROGRAMS = test_authorize  \
                 test_clients    \
                 test_helpers    \
                 test_identities \
                 test_introspect \
                 test_json       \
                 test_rest       \
                 test_scopes     \
                 test_token      \
                 test_userinfo

LDADD = ../libglobus_auth.la $(CMOCKA_LIBS)
#LDADD = $(CURL_LIBS) $(SSL_LIBS) $(JSONC_LIBS) $(CMOCKA_LIBS)

LIB_SOURCES=$(realpath ../authorize.c)  \
            $(realpath ../clients.c)    \
            $(realpath ../helpers.c)    \
            $(realpath ../identities.c) \
            $(realpath ../introspect.c) \
            $(realpath ../json.c)       \
            $(realpath ../rest.c)       \
            $(realpath ../scopes.c)     \
            $(realpath ../token.c)      \
            $(realpath ../userinfo.c)


test_authorize_SOURCES  = test_authorize.c $(LIB_SOURCES)
test_clients_SOURCES    = test_clients.c $(LIB_SOURCES)
test_helpers_SOURCES    = test_helpers.c $(LIB_SOURCES)
test_identities_SOURCES = test_identities.c $(LIB_SOURCES)
test_introspect_SOURCES = test_introspect.c $(LIB_SOURCES)
test_json_SOURCES       = test_json.c $(LIB_SOURCES)
test_rest_SOURCES       = test_rest.c $(LIB_SOURCES)
test_scopes_SOURCES     = test_scopes.c $(LIB_SOURCES)
test_token_SOURCES      = test_token.c $(LIB_SOURCES)
test_userinfo_SOURCES   = test_userinfo.c $(LIB_SOURCES)

AM_CFLAGS = -DUNIT_TEST
AM_CPPFLAGS = -I.. $(CMOCKA_CFLAGS)

