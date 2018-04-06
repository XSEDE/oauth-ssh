##
## Copyright The University of Chicago
## All Rights Reserved.
##
##SUBDIRS = utils tests
SUBDIRS = . utils

ACLOCAL_AMFLAGS = -I m4

noinst_LTLIBRARIES = libglobus_auth.la
libglobus_auth_la_SOURCES = auth_error.c      \
                            auth_introspect.c \
                            curl_error.c      \
                            curl_wrapper.c    \
                            endpoints.c       \
                            endpoint_errors.c \
                            globus_auth.c     \
                            json.c            \
                            kv.c              \
                            response.c        \
                            request.c         \
                            strings.c
libglobus_auth_la_LIBADD = $(CURL_LIBS) $(SSL_LIBS) $(JSONC_LIBS)

AM_CPPFLAGS = -std=c99 -pedantic-errors -D_POSIX_C_SOURCE=200809L -DDEBUG -D_GNU_SOURCE
