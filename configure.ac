## @file configure.ac
## @author Tim van Werkhoven <t.i.m.vanwerkhoven@xs4all.nl>

AC_INIT([ICS2FITS], [0.1], [t.i.m.vanwerkhoven@xs4all.nl])
AC_PREREQ(2.59)
AC_COPYRIGHT([Copyright (C) 2010 Tim van Werkhoven <t.i.m.vanwerkhoven@xs4all.nl>])

AC_CONFIG_AUX_DIR([build-aux])

AC_CONFIG_SRCDIR([src/ics2fits.cc])

AC_CONFIG_FILES([
Makefile
src/Makefile
])

AM_INIT_AUTOMAKE([-Wall -Werror foreign])

AC_PROG_CXX
AC_PROG_CXXCPP
AC_PROG_MAKE_SET
AC_PROG_INSTALL

# These libraries are crucial for all targets
AC_SEARCH_LIBS([deflate],
		[z],
		[],
		[AC_MSG_ERROR([Error, need libgz.])])


AC_SEARCH_LIBS([IcsOpen],
		[ics],
		[],
		[AC_MSG_ERROR([Error, cannot use IcsOpen(). Please make sure libics is installed])])

AC_SEARCH_LIBS([ffopen],
		[cfitsio],
		[],
		[AC_MSG_ERROR([Error, cannot use ffopen(). Please make sure libcfitsio is installed])])

AC_OUTPUT
