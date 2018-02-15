dnl
dnl Copyright The University of Chicago
dnl All Rights Reserved.
dnl

AC_PREREQ([2.69])
AC_INIT([SSH With Globus Auth], [0.1], [support@globus.org])

AC_CONFIG_MACRO_DIR([m4])

PKG_CHECK_MODULES([CMOCKA], [cmocka], [], [])
PKG_CHECK_MODULES([CURL],   [libcurl],[], [])
PKG_CHECK_MODULES([SSL],    [libssl], [], [])
PKG_CHECK_MODULES([JSONC],  [json-c], [], [])

AC_CHECK_HEADERS([security/pam_modules.h], [], [AC_MSG_ERROR([missing PAM headers])])
save_LIBS=$LIBS
AC_SEARCH_LIBS(pam_start, [pam], [AC_SUBST(PAM_LIBS, [-lpam])], [AC_MSG_ERROR([libpam not found])])
LIBS=$save_LIBS

AC_PROG_CC
AC_PROG_CC_STDC

AM_INIT_AUTOMAKE([foreign])
AM_PROG_AR

LT_INIT

AC_CONFIG_FILES([Makefile 
                 account_mapping/Makefile 
                 account_mapping/tests/Makefile 
                 globus_auth/Makefile 
                 pam/Makefile 
                 pam/test/Makefile 
                 globus_auth/utils/Makefile
                 globus_auth/tests/Makefile])
AC_OUTPUT
