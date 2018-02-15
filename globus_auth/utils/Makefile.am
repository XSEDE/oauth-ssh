##
## Copyright The University of Chicago
## All Rights Reserved.
##
ACLOCAL_AMFLAGS = -I m4
AM_CFLAGS=-I..

noinst_PROGRAMS  = tokens     \
                   scopes     \
                   clients    \
                   identities

#                   authorize  \
#                   userinfo

#authorize_SOURCES = authorize.c
#authorize_LDADD = ../libglobus_auth.la

clients_SOURCES = clients.c
clients_LDADD = ../libglobus_auth.la

identities_SOURCES = identities.c
identities_LDADD = ../libglobus_auth.la

scopes_SOURCES = scopes.c
scopes_LDADD = ../libglobus_auth.la

tokens_SOURCES = tokens.c
tokens_LDADD = ../libglobus_auth.la

#userinfo_SOURCES = userinfo.c
#userinfo_LDADD = ../libglobus_auth.la
