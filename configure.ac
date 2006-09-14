dnl -----------------------------------------------------------------------
dnl Stage Autoconf script
dnl $Id: configure.ac,v 1.13.2.2 2006-09-14 07:20:28 rtv Exp $
dnl -----------------------------------------------------------------------

dnl AC_INIT( package, version )
AC_INIT(stage,2.1.0)

AC_CONFIG_SRCDIR(src/stage.c)

dnl determine system type.  this is used in a couple of places to infer the 
dnl types of arguments to some networking library functions.  if/when we 
dnl upgrade to newer Autotools, this can probably go away.
AC_CANONICAL_SYSTEM

dnl Initialize automake, requiring version 1.9 or better,
dnl using modern tar format and generate bzip tarballs
AM_INIT_AUTOMAKE([tar-ustar dist-bzip2 1.9])

dnl Need AC_PROG_MAKE_SET for recursive building
AC_PROG_MAKE_SET

dnl Generate a configuration header called <config.h> and put all the C
dnl preprocessor defines in there
AC_CONFIG_HEADERS(config.h)

dnl Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL

dnl tell libtool to enable dynamic-loadable libraries
AC_LIBTOOL_DLOPEN
dnl enable libtool for building shared libraries in a platform-independent way
AC_PROG_LIBTOOL
AC_SUBST(LIBTOOL_DEPS)

dnl Checks for header files of required libraries
AC_CHECK_HEADERS(stdint.h strings.h)

dnl build mplementations of these in libreplace.a if these are missing
AC_REPLACE_FUNCS(dirname scandir basename)

dnl Do we have pkg-config?
AC_CHECK_PROG(have_pkg_config,pkg-config,yes,no)
if test "x$have_pkg_config" = "xno"; then
  AC_MSG_ERROR([pkg-config is required to build Stage.  Install pkg-config, then run configure again.])
fi

dnl ==== PLAYER ====

dnl add prefix to the pkg-config path in case things were installed there
dnl and the  user didn't add it manually
export PKG_CONFIG_PATH=$prefix/lib/pkgconfig:$PKG_CONFIG_PATH

dnl for libstageplugin, we need Player 2.0 or higher
PKG_CHECK_MODULES( PLAYER, playercore >= 2.0.1,
 AC_DEFINE(INCLUDE_PLAYER,1,[building Player plugin])
 have_player=yes,
 AC_MSG_WARN([No suitable Player installation detected.])
 have_player=no
)

AM_CONDITIONAL(HAVE_PLAYER, [test $have_player=yes])

dnl for ptest, we need libplayerc
dnl PKG_CHECK_MODULES( PLAYERC, playerc >= 2.0,
dnl  AC_DEFINE(INCLUDE_LIBPLAYERC,1,[building test Player client]),
dnl  AC_MSG_WARN([libplayercr was not found. ])
dnl )

dnl ==== GUI/GRAPHICS ====

dnl gtkglext brings in all dependencies on GTK, glib, etc. Almost too easy!
PKG_CHECK_MODULES(GUI, gtkglext-1.0 >= 1.0, , 
 AC_MSG_ERROR([Required library GtkGLExt not found. Install it and try again.]) 
)

dnl TODO - use Reed Hedges' MobileSim patch to improve on this
dnl where's the X11 color database?
AC_CHECK_FILE([/usr/X11R6/lib/X11/rgb.txt],
  CDB="\"/usr/X11R6/lib/X11/rgb.txt\"",
  AC_CHECK_FILE([/usr/openwin/lib/X11/rgb.txt],
    CDB="\"/usr/openwin/lib/X11/rgb.txt\"",
    CDB="\"/usr/X11R6/lib/X11/rgb.txt\""))
AC_ARG_WITH(color, [   --with-color=file      Location of X11-style color file],
CDB="\"$with_color\"",)
AC_DEFINE_UNQUOTED(COLOR_DATABASE,$CDB,[the X11-style color database])

dnl ==== FINISH UP =====

AC_OUTPUT(Makefile 
          src/Makefile 
	  docsrc/Makefile
          worlds/Makefile 
          worlds/bitmaps/Makefile 
          replace/Makefile
          src/referees/Makefile
	  stage.pc )

dnl should use AC_MSG_NOTICE(), but that's apparently not defined in
dnl autoconf 2.13.
AC_MSG_RESULT([])
AC_MSG_RESULT([*************************************************************])
AC_MSG_RESULT([])
AC_MSG_RESULT([Stage will be built for a $host system, with the following tools and options:])
AC_MSG_RESULT([  C   compiler: $CC $CFLAGS ])
AC_MSG_RESULT([  C++ compiler: $CXX $CXXFLAGS ])
AC_MSG_RESULT([])

if test $have_player = yes; then
  AC_MSG_RESULT([The Player plugin (libstageplugin) will be built using the following flags: ])
  AC_MSG_RESULT([  compiler: $PLAYER_CFLAGS])
  AC_MSG_RESULT([    linker: $PLAYER_LIBS])
else
  AC_MSG_RESULT([Player was not found, so the Stage plugin for Player will NOT be built.])
  AC_MSG_RESULT([If you want the Player/Stage system, you must install Player and configure Stage again.])
fi

AC_MSG_RESULT([])
AC_MSG_RESULT([Stage will be installed in:])
AC_MSG_RESULT([    $prefix/])
AC_MSG_RESULT([])
AC_MSG_RESULT([To see the configuration options, do:])
AC_MSG_RESULT([    ./configure --help])
AC_MSG_RESULT([])
AC_MSG_RESULT([When you're satisfied with the configuration, type 'make install'. ])
AC_MSG_RESULT([])
