##
## Copyright The University of Chicago
## All Rights Reserved.
##

ACLOCAL_AMFLAGS = -I m4
AM_CPPFLAGS=-I$(srcdir)/..

noinst_PROGRAMS  = get_identities introspect_token

get_identities_SOURCES = get_identities.c \
                         ../config.c      \
                         ../credentials.c \
                         ../http.c        \
                         ../identities.c  \
                         ../json.c        \
                         ../strings.c 

LDADD = -ljson-c -lcurl

introspect_token_SOURCES = introspect_token.c \
                           ../config.c      \
                           ../credentials.c \
                           ../http.c        \
                           ../introspect.c  \
                           ../json.c        \
                           ../strings.c 
