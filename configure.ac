# *****************************************************************************
# Versioning
# *****************************************************************************

m4_define([ghwp_major_version],[0])
m4_define([ghwp_minor_version],[2])
m4_define([ghwp_micro_version],[])
m4_define([ghwp_extra_version],[])
m4_define([ghwp_version],[ghwp_major_version.ghwp_minor_version.ghwp_micro_version()ghwp_extra_version])

# Libtool versioning. The backend and view libraries have separate versions.
# Before making a release, the libtool version should be modified.
# The string is of the form C:R:A.
# - If interfaces have been changed or added, but binary compatibility has
#   been preserved, change to C+1:0:A+1
# - If binary compatibility has been broken (eg removed or changed interfaces)
#   change to C+1:0:0
# - If the interface is the same as the previous version, change to C:R+1:A

# Libtool version
m4_define([ghwp_lt_current], [1])
m4_define([ghwp_lt_revision],[0])
m4_define([ghwp_lt_age],     [0])
m4_define([ghwp_lt_version_info],[ghwp_lt_current:ghwp_lt_revision:ghwp_lt_age])
m4_define([ghwp_lt_current_minus_age],[m4_eval(ghwp_lt_current - ghwp_lt_age)])

# *****************************************************************************

AC_INIT([libghwp], [ghwp_version],
        [https://github.com/cogniti/libghwp/issues], [libghwp],
        [https://github.com/cogniti/libghwp])

AC_CONFIG_HEADERS([config.h])

AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([1.11])

AM_SILENT_RULES([yes])

AC_PROG_CC
AC_PROG_CC_C99

LT_INIT

dnl ***************************************************************************
dnl Internationalization
dnl ***************************************************************************
IT_PROG_INTLTOOL([0.35.0])

GETTEXT_PACKAGE=libghwp
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE],["$GETTEXT_PACKAGE"],[Gettext package])
AM_GLIB_GNU_GETTEXT

PKG_CHECK_MODULES(GHWP, [libgsf-1 glib-2.0 gio-2.0 cairo gobject-2.0 cairo-ft freetype2 libxml-2.0 fontconfig gdk-pixbuf-2.0])

dnl gsf_msole_metadata_read is deprecated since libgsf 1.14.24
dnl check if your libgsf-1 have gsf_doc_meta_data_read_from_msole
AC_CHECK_LIB(gsf-1, gsf_doc_meta_data_read_from_msole,
             AC_DEFINE(HAVE_GSF_DOC_META_DATA_READ_FROM_MSOLE,
                       [1],
                       [Define to 1 if your libgsf-1 have gsf_doc_meta_data_read_from_msole.]))

# **********
# Versioning
# **********

AC_SUBST([GHWP_MAJOR_VERSION],[ghwp_major_version])
AC_SUBST([GHWP_MINOR_VERSION],[ghwp_minor_version])
AC_SUBST([GHWP_MICRO_VERSION],[ghwp_micro_version])

AC_SUBST([GHWP_LT_VERSION_INFO],[ghwp_lt_version_info])
AC_SUBST([GHWP_LT_CURRENT_MINUS_AGE],[ghwp_lt_current_minus_age])

# **********************************

AC_OUTPUT([
    Makefile
    po/Makefile.in
    src/ghwp-0.2.pc
    src/Makefile
    src/ghwp-version.h
])
