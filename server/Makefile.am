##
## Copyright The University of Chicago
## All Rights Reserved.
##
SUBDIRS = src/pam src/config man/

globusconfdir = $(sysconfdir)/globus
globusconf_DATA = data/globus-ssh.conf data/globus-acct-map

doc_DATA = data/globus-ssh.te

EXTRA_DIST = $(globusconf_DATA) $(doc_DATA)

ACLOCAL_AMFLAGS = -I m4
