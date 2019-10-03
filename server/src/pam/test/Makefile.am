
TESTS = test_account_map \
        test_base64 \
        test_identities \
        test_introspect \
        test_json \
        test_parser \
        test_strings

check_PROGRAMS= $(TESTS)

AM_CPPFLAGS = $(CMOCKA_CFLAGS) -I$(srcdir)/..
AM_LDFLAGS= -Wl,-rpath ../.libs -rdynamic # so we can override internal symbols
LDADD=$(CMOCKA_LIBS) ../.libs/pam_oauth_ssh.so -ldl -lpthread -lpam

COMMON_SOURCES = debug.h \
                 debug.c

test_account_map_SOURCES = test_account_map.c $(COMMON_SOURCES)
test_base64_SOURCES = test_base64.c $(COMMON_SOURCES)
test_identities_SOURCES = test_identities.c $(COMMON_SOURCES)
test_introspect_SOURCES = test_introspect.c $(COMMON_SOURCES)
test_json_SOURCES = test_json.c $(COMMON_SOURCES)
test_parser_SOURCES = test_parser.c $(COMMON_SOURCES)
test_strings_SOURCES = test_strings.c $(COMMON_SOURCES)
