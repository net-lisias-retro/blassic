# configure.ac for blassic

# Require autoconf >= 2.50
AC_PREREQ(2.50)
AC_INIT(version.cpp)
AC_CANONICAL_HOST
AC_CANONICAL_SYSTEM


# Parse version.cpp to get the version number.
# The number is defined in version.cpp to avoid affecting the Windows
# compilation process.
#
# Julian: changed the "grep -w Major" for "grep 'version::Major'"
# because the -w option in not always present,

AC_MSG_CHECKING(for version number in $srcdir/version.cpp)

VERSION_MAJOR=`grep 'version::Major' $srcdir/version.cpp | \
				sed 's/.*version::.*= *//;s/[[,;]]//'`
VERSION_MINOR=`grep 'version::Minor' $srcdir/version.cpp | \
				sed 's/.*version::.*= *//;s/[[,;]]//'`
VERSION_RELEASE=`grep 'version::Release' $srcdir/version.cpp | \
				sed 's/.*version::.*= *//;s/[[,;]]//'`

AC_MSG_RESULT([got $VERSION_MAJOR.$VERSION_MINOR.$VERSION_RELEASE])

if test "_$VERSION_MAJOR" = "_"; then
	AC_MSG_ERROR([version::Major not found])
elif test "_$VERSION_MINOR" = "_"; then
	AC_MSG_ERROR([version::Minor not found])
elif test "_$VERSION_RELEASE" = "_"; then
	AC_MSG_ERROR([version::Release not found])
fi

AM_INIT_AUTOMAKE(blassic, $VERSION_MAJOR.$VERSION_MINOR.$VERSION_RELEASE)


# Checks for programs.
AC_PROG_CXX
AC_LANG_CPLUSPLUS


#CXXFLAGS="-g"  # default compilation options; avoids having -O2 by default
CXXFLAGS="-O3"	# We want maximun optimization even in debugging.

# Julian: Some systems install ncurses in /opt, path to X libraries needed.
# Julian: and some does not have the X includes default accesible.
CPPFLAGS="-I/opt/ncurses/include -I/usr/X11R6/include"
LDFLAGS="-L/opt/ncurses/lib -L/usr/X11R6/lib"

# Julian: under cygwin and mingw, link with gdi32.dll and wsock32.dll
case "$host" in
*-*-cygwin*|*mingw*)
	#LDFLAGS=-lgdi32
	AC_SUBST(CYGWIN_FLAGS,"-lgdi32 -lwsock32")
	;;
*)
	AC_SUBST(CYGWIN_FLAGS)
	;;
esac

###############################################################################
# Julian
# If the systems has libdl, use it.

AC_CHECK_LIB(dl, dlopen)

# Check X11 lib, mainly to not try to link it when using cygwin.

AC_CHECK_LIB([X11], [XCreateWindow])

# Check for ncurses, if not found for curses.

AC_CHECK_LIB(ncurses,tputs,,AC_CHECK_LIB(curses,tputs) )

###############################################################################


AC_ARG_ENABLE(svgalib,
	[  --enable-svgalib              Use svgalib [[yes]]],
	[
		if test "_$enableval" = "_no"; then
			:
			AC_SUBST(SVGALIB_CFLAGS)
			AC_SUBST(SVGALIB_LIBS)
		else
			AM_PATH_SVGALIB(1.4.0)
		fi
	],
	[
		:
		AC_SUBST(SVGALIB_CFLAGS)
		AC_SUBST(SVGALIB_LIBS)
	]
)
if test "_$SVGALIB_LIBS" = "_"; then
	AC_MSG_RESULT([not using svgalib])
fi


###############################################################################


AC_ARG_ENABLE(installed-examples,
	[  --enable-installed-examples   Install the example programs [[yes]]],
	[
		if test "_$enableval" = "_no"; then
			INSTALL_EXAMPLE_PROGS=no
		else
			INSTALL_EXAMPLE_PROGS=yes
		fi
	],
	[
		INSTALL_EXAMPLE_PROGS=yes
	]
)

AM_CONDITIONAL(INSTALL_EXAMPLE_PROGS, [test "_$INSTALL_EXAMPLE_PROGS" = _yes])




###############################################################################

# Generate files.
AC_OUTPUT([
	Makefile
	blassic.spec
])
