##
## Copyright The University of Chicago
## All Rights Reserved.
##
SUBDIRS = src

globusconfdir = $(sysconfdir)/globus
globusconf_DATA = globus-ssh.conf globus-acct-map

doc_DATA = globus-ssh.pp

EXTRA_DIST = $(globusconf_DATA) $(doc_DATA) globus-ssh.te

ACLOCAL_AMFLAGS = -I m4

globus-ssh.pp: globus-ssh.te
	checkmodule -M -m -o globus-ssh.mod globus-ssh.te
	semodule_package -o globus-ssh.pp -m globus-ssh.mod

#clean:
#	rm -f globus-ssh.pp globus-ssh.mod

