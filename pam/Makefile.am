##
## Copyright The University of Chicago
## All Rights Reserved.
##
SUBDIRS = . test

ACLOCAL_AMFLAGS = -I m4
AM_CPPFLAGS = -I../globus_auth -I../account_mapping -I..

lib_LTLIBRARIES    = pam_globus.la
pam_globus_la_LDFLAGS = -module -shared
pam_globus_la_SOURCES = pam.c credentials.c

pam_globus_la_LIBADD = ../account_mapping/libaccount_mapping.la \
                       ../globus_auth/libglobus_auth.la \
                       -lpam -lcurl -lssl -lcrypto -ljson-c

