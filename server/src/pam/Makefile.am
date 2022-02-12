SUBDIRS = . test

ACLOCAL_AMFLAGS = -I m4

lib_LTLIBRARIES    = pam_oauth_ssh.la
pam_oauth_ssh_la_LDFLAGS = -shared -module -avoid-version
pam_oauth_ssh_la_LIBADD  = -lssl -ljson-c -lcurl -lpthread
pam_oauth_ssh_la_SOURCES = account_map.c \
                        account_map.h \
                        base64.c \
                        base64.h \
                        client.c \
                        client.h \
                        config.c \
                        config.h \
                        debug.c \
                        debug.h \
                        globus_auth.c \
                        globus_auth.h \
                        http.c \
                        http.h \
                        identities.c \
                        identities.h \
                        introspect.c \
                        introspect.h \
                        json.c \
                        json.h \
                        logger.c \
                        logger.h \
                        pam.c \
                        parser.c \
                        parser.h \
                        strings.c \
                        strings.h
if WITH_SCITOKENS
pam_oauth_ssh_la_SOURCES += scitokens_verify.c \
                        scitokens_verify.h
endif
