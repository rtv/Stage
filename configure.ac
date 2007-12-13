dnl -----------------------------------------------------------------------
dnl Stage Autoconf script
dnl $Id: configure.ac,v 1.17 2007-12-13 20:31:07 gerkey Exp $
dnl -----------------------------------------------------------------------

dnl AC_INIT( package, version )
AC_INIT(stage,2.1.0rc1)

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

dnl add prefix to the pkg-config path in case things were installed there
dnl and the  user didn't add it manually
export PKG_CONFIG_PATH=$prefix/lib/pkgconfig:$PKG_CONFIG_PATH

dnl check for GTK stuff - we need it for the GUI
PKG_CHECK_MODULES(GTK, gtk+-2.0 >= 2.4, ,
 AC_MSG_WARN([A suitable GTK+ was not found. GTK+-2.4 or later is required. ])
)

	
dnl if GNOMECANVAS is requested and available, we use that for graphics
AC_ARG_ENABLE(gnomecanvas,
	      AC_HELP_STRING([--enable-gnomecanvas],[Use GnomeCanvas if available *EXPERIMENTAL (rtv)*]),
	      [ if test "x$enableval" = "xyes"; then
	      PKG_CHECK_MODULES(GNOME, libgnomecanvas-2.0 >= 2.0,
	        gnome=true
                AC_DEFINE(INCLUDE_GNOME,1,[building GnomeCanvas components]),
	        AC_MSG_WARN([no suitable GnomeCanvas library was found. ])
	        )
 	        AC_MSG_RESULT([GnomeCanvas graphics will be used.])
              else
                gnome=false
	        AC_MSG_RESULT([GnomeCanvas graphics disabled by user])
              fi ],
)

AM_CONDITIONAL(GNOME, test x$gnome = xtrue)

dnl for libstageplugin, we need Player 2.0 or higher
PKG_CHECK_MODULES( PLAYER, playercore >= 2.0.1,
 AC_DEFINE(INCLUDE_PLAYER,1,[building Player plugin])
 have_player=yes
 ,
 AC_MSG_WARN([No suitable Player installation detected.])
 have_player=no
)

AM_CONDITIONAL(HAVE_PLAYER, [test $have_player = yes])


dnl for libplayerc examples, we need libplayerc
PKG_CHECK_MODULES( PLAYERC, playerc >= 2.0.1,
 AC_DEFINE(INCLUDE_PLAYERC,1,[building libplayerc examples])
 have_playerc=yes
 ,
 AC_MSG_WARN([libplayerc was not found. ])
 have_playerc=no
)

AM_CONDITIONAL(HAVE_PLAYERC, [test $have_playerc = yes])

dnl where's the X11 color database?
AC_CHECK_FILE([/usr/X11R6/lib/X11/rgb.txt],
  CDB="\"/usr/X11R6/lib/X11/rgb.txt\"",
  AC_CHECK_FILE([/usr/openwin/lib/X11/rgb.txt],
    CDB="\"/usr/openwin/lib/X11/rgb.txt\"",
    AC_CHECK_FILE([/usr/share/X11/rgb.txt],
      CDB="\"/usr/share/X11/rgb.txt\"",
      CDB="\"/usr/X11R6/lib/X11/rgb.txt\"")))
AC_ARG_WITH(color, [   --with-color=file      Location of X11-style color file],
CDB="\"$with_color\"",)
AC_DEFINE_UNQUOTED(COLOR_DATABASE,$CDB,[the X11-style color database])

AC_OUTPUT(Makefile 
          src/Makefile 
	  docsrc/Makefile
          worlds/Makefile 
          worlds/wifi/Makefile 
          worlds/bitmaps/Makefile 
          replace/Makefile
          examples/Makefile
          examples/libstage/Makefile
          examples/libplayerc/Makefile
	  stage.pc )

dnl should use AC_MSG_NOTICE(), but that's apparently not defined in
dnl autoconf 2.13.
AC_MSG_RESULT([])
AC_MSG_RESULT([*************************************************************])
AC_MSG_RESULT([])
AC_MSG_RESULT([Stage will be built for a $host system, with the following tools and options:])
AC_MSG_RESULT([  C   compiler: $CC $CFLAGS])
AC_MSG_RESULT([  C++ compiler: $CXX $CXXFLAGS])
AC_MSG_RESULT([])

if test "x$have_player" = "xyes"; then
  AC_MSG_RESULT([The Player plugin (libstageplugin) will be built using the following flags: ])
  AC_MSG_RESULT([  compiler: $PLAYER_CFLAGS])
  AC_MSG_RESULT([    linker: $PLAYER_LIBS])
  AC_MSG_RESULT([])
else
  AC_MSG_RESULT([Player was not found, so the Stage plugin for Player will NOT be built.])
  AC_MSG_RESULT([If you want the Player/Stage system, you must install Player and configure Stage again.])
  AC_MSG_RESULT([])
fi

if test "x$have_playerc" = "xyes"; then
  AC_MSG_RESULT([libplayerc examples will be built using the following flags: ])
dnl  AC_MSG_RESULT([  compiler: $PLAYERC_CFLAGS])
dnl  AC_MSG_RESULT([    linker: $PLAYERC_LIBS])
  AC_MSG_RESULT([])
else
  AC_MSG_RESULT([libplayerc was not found, so the the examples will NOT be built.])
  AC_MSG_RESULT([])
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
