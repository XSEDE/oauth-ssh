#
# Copyright The University of Chicago
# All Rights Reserved.
#

# Single component Unit Tests
UNIT_TESTS  = unit_acct_map \
              unit_strings  \
              unit_acct_map_module_idp \
              unit_acct_map_module_file \
              unit_config \
              unit_credentials \
              unit_http \
              unit_json \
              unit_pam_utils \
              unit_scope \
              unit_introspect \
              unit_identities

# Combined unit integration tests
INTEG_TESTS = integ_pam_module_load

# Single and combined 'live' tests
FUNC_TESTS  = func_pam

TESTS = $(UNIT_TESTS) $(INTEG_TESTS)

check_PROGRAMS= $(UNIT_TESTS) $(INTEG_TESTS) $(FUNC_TESTS)

AM_CFLAGS = -DUNIT_TESTING $(CMOCKA_CFLAGS)
LDADD=$(CMOCKA_LIBS)

unit_acct_map_SOURCES = unit_acct_map.c unit_test.c unit_test.h mock_acct_map_module.h
unit_acct_map_CPPFLAGS = $(AM_CPPFLAGS) -I$(srcdir)/..
unit_acct_map_module_idp_SOURCES = unit_acct_map_module_idp.c unit_test.c unit_test.h
unit_acct_map_module_file_SOURCES = unit_acct_map_module_file.c unit_test.c unit_test.h mock_config.h

unit_config_SOURCES = unit_config.c unit_test.c unit_test.h mock_stdio.h
unit_config_LDFLAGS= -Wl,--wrap=fopen -Wl,--wrap=fclose -Wl,--wrap=fgets

unit_credentials_SOURCES = unit_credentials.c unit_test.c unit_test.h

unit_http_SOURCES = unit_http.c unit_test.c ../strings.c unit_test.h
unit_http_LDADD = -lcurl $(LDADD)

unit_identities_SOURCES = unit_identities.c unit_test.c unit_test.h mock_http.h
unit_identities_LDADD = $(unit_json_LDADD)

unit_introspect_SOURCES = unit_introspect.c unit_test.c unit_test.h mock_http.h
unit_introspect_LDADD = $(unit_json_LDADD)

unit_json_SOURCES = unit_json.c unit_test.c unit_test.h
unit_json_LDADD = -ljson-c $(LDADD)

unit_pam_utils_SOURCES = unit_pam_utils.c unit_test.c unit_test.h ../pam_utils.c

unit_scope_SOURCES  = unit_scope.c unit_test.c unit_test.h
unit_strings_SOURCES = unit_strings.c unit_test.c unit_test.h

integ_pam_module_load_SOURCES = integ_pam_module_load.c unit_test.c unit_test.h
integ_pam_module_load_LDADD = -ldl $(LDADD)

func_pam_SOURCES = func_pam.c
func_pam_LDADD = ../.libs/pam_globus.so -lpam
