SUBDIRS = src/pam src/config man/

oauthsshconfdir = $(sysconfdir)/oauth_ssh
oauthsshconf_DATA = data/oauth-ssh.conf data/globus-acct-map

doc_DATA = data/oauth-ssh.te

EXTRA_DIST = $(oauthsshconf_DATA) $(doc_DATA)

ACLOCAL_AMFLAGS = -I m4
