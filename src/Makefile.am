##
## Copyright The University of Chicago
## All Rights Reserved.
##
SUBDIRS = .

if UNIT_TEST
        SUBDIRS += test
endif


ACLOCAL_AMFLAGS = -I m4

lib_LTLIBRARIES    = pam_globus.la
pam_globus_la_LDFLAGS = -module -shared
pam_globus_la_SOURCES = acct_map.c \
                        acct_map_module_file.c \
                        acct_map_module_idp.c \
                        config.c \
                        credentials.c \
                        http.c \
                        identities.c \
                        introspect.c \
                        json.c \
                        pam.c \
                        pam_utils.c \
                        scope.c \
                        strings.c \
			acct_map.h \
			acct_map_module.h \
			acct_map_module_file.h \
			acct_map_module_idp.h \
			auth.h \
			config.h \
			credentials.h \
			http.h \
			identities.h \
			introspect.h \
			json.h \
			scope.h \
			strings.h

pam_globus_la_LIBADD = -lpam -ljson-c -lcurl
