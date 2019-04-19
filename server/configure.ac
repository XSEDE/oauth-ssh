AC_PREREQ([2.69])
AC_INIT([oauth_ssh], [0.9])

AC_CONFIG_MACRO_DIR([m4])

PKG_CHECK_MODULES([CURL],   [libcurl],[], [])
PKG_CHECK_MODULES([SSL],    [libssl], [], [])
PKG_CHECK_MODULES([JSONC],  [json-c], [], [])
PKG_CHECK_MODULES([CMOCKA], [cmocka], [], [AC_MSG_WARN([skipping tests])])
AM_CONDITIONAL([UNIT_TEST], [test ! -z ${CMOCKA_LIBS+x}])

AC_CHECK_HEADERS([security/pam_modules.h], [], [AC_MSG_ERROR([missing PAM headers])])
save_LIBS=$LIBS
AC_SEARCH_LIBS(pam_start, [pam], [AC_SUBST(PAM_LIBS, [-lpam])], [AC_MSG_ERROR([libpam not found])])
LIBS=$save_LIBS

AC_PROG_CC
AC_PROG_CC_STDC

AM_INIT_AUTOMAKE([foreign subdir-objects])
AM_PROG_AR

LT_INIT

AC_CONFIG_FILES(Makefile 
                src/pam/Makefile 
                src/pam/test/Makefile 
                src/config/Makefile
                src/config/setup.py
                packaging/fedora/oauth-ssh.spec
                man/Makefile)

AC_OUTPUT
