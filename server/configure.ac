AC_PREREQ([2.69])
AC_INIT([oauth_ssh], [0.11])

AC_CONFIG_MACRO_DIR([m4])

PKG_CHECK_MODULES([CURL],   [libcurl],[], [])
PKG_CHECK_MODULES([SSL],    [libssl], [], [])
PKG_CHECK_MODULES([JSONC],  [json-c], [], [])
PKG_CHECK_MODULES([CMOCKA], [cmocka], [], [])

AC_ARG_ENABLE(scitokens, [AS_HELP_STRING([--enable-scitokens],[Enable SciTokens support for authorization (default is no)])],
 [enable_scitokens="yes"],
 [enable_scitokens="no"])

AC_MSG_RESULT(checking if SciTokens are enabled... $enable_scitokens)
AS_IF([test "$enable_scitokens" = "yes"],[
   AC_CHECK_LIB(SciTokens,scitoken_create,[],[AC_MSG_ERROR([Missing SciTokens library])])
   AC_CHECK_HEADERS(scitokens/scitokens.h,[],[AC_MSG_ERROR([Missing scitokens.h header file])])
   AC_DEFINE(WITH_SCITOKENS)
])

AM_CONDITIONAL([WITH_SCITOKENS], [test "$enable_scitokens" = "yes"])

AC_CHECK_HEADERS([security/pam_modules.h], [], [AC_MSG_ERROR([missing PAM headers])])
save_LIBS=$LIBS
AC_SEARCH_LIBS(pam_start, [pam], [AC_SUBST(PAM_LIBS, [-lpam])], [AC_MSG_ERROR([libpam not found])])
LIBS=$save_LIBS

AC_PROG_CC
AC_PROG_CC_STDC

AM_INIT_AUTOMAKE([foreign subdir-objects])
AM_PROG_AR

LT_INIT

AC_SUBST(PACKAGE_RELEASE_TAG, '')
# This next line is modified by Makefile.bootstrap to mark development releases
##develop AC_SUBST(PACKAGE_RELEASE_TAG, .dev)

AC_CONFIG_FILES(Makefile
                src/pam/Makefile
                src/pam/test/Makefile
                src/config/Makefile
                src/config/setup.cfg
                packaging/fedora/oauth-ssh.spec
                man/Makefile)

AC_OUTPUT
