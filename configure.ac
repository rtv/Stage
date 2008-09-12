dnl -----------------------------------------------------------------------
dnl Stage Autoconf script
dnl $Id: configure.ac,v 1.24 2008-02-24 20:46:12 rtv Exp $
dnl -----------------------------------------------------------------------

dnl AC_INIT( package, version )
AC_INIT(Stage,3.0.0)  
			
dnl Check for a source file, to make sure any user-specified source
dnl dir is correct	 	
AC_CONFIG_SRCDIR(libstage/model.cc)

dnl Determine system type. We need to know if we're on Darwin so we
dnl can make spacial arrangements for OS X's weird framework system
AC_CANONICAL_SYSTEM
darwin=no
case $host in
  *-darwin*)
    darwin=yes ;;
esac
AM_CONDITIONAL(DARWIN, test "x$darwin" = "xyes")


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

dnl ==== OPEN GL =====
dnl AX_CHECK_GLU

dnl ==== GRAPHICS FILES ====

PKG_CHECK_MODULES(PIXBUF, gdk-pixbuf-2.0 >= 1.0, , 
  AC_MSG_ERROR([Required library gdk-pixbuf-2.0 not found. Install it and try again.]) 
)

dnl ==== FLTK GUI ====

AC_CHECK_PROGS(HAVE_FLTK_CONFIG, fltk-config, "")
if test -n "$HAVE_FLTK_CONFIG"; then
  AC_SUBST(FLTK_CFLAGS,"`fltk-config --use-gl --use-images --cxxflags`")
  AC_SUBST(FLTK_LIBS,"`fltk-config --use-gl --use-images --ldflags`")
else
 AC_MSG_ERROR(Could not find fltk-config. Check that FLTK is installed.)
fi

dnl ==== PLAYER ====

dnl add prefix to the pkg-config path in case things were installed there
dnl and the  user didn't add it manually
export PKG_CONFIG_PATH=$prefix/lib/pkgconfig:$PKG_CONFIG_PATH

dnl Disabling this check for now, only on the stage-ros branch
dnl for libstageplugin, we need Player 2.1 or higher
dnl PKG_CHECK_MODULES( PLAYER, playercore >= 2.1.0,
dnl  AC_DEFINE(INCLUDE_PLAYER,1,[building Player plugin])
dnl  have_player=yes,
dnl  AC_MSG_WARN([No suitable Player installation detected.])
dnl  have_player=no
dnl )
have_player=no
AM_CONDITIONAL(HAVE_PLAYER, test "x$have_player" = "xyes")


dnl for libplayerc examples, we need libplayerc
dnl PKG_CHECK_MODULES( PLAYERC, playerc >= 2.0.1,
dnl  AC_DEFINE(INCLUDE_PLAYERC,1,[building libplayerc examples])
dnl  have_playerc=yes
dnl  ,
dnl  AC_MSG_WARN([libplayerc was not found. ])
dnl  have_playerc=no
dnl )
have_playerc=no
AM_CONDITIONAL(HAVE_PLAYERC, test "x$have_playerc" = "xyes")

AC_OUTPUT(Makefile 
          libstage/Makefile 
          libstageplugin/Makefile 
	  docsrc/Makefile
          worlds/Makefile 
          worlds/wifi/Makefile 
          worlds/bitmaps/Makefile 
          replace/Makefile
          examples/Makefile
          examples/libstage/Makefile
          examples/libplayerc/Makefile
          examples/ctrl/Makefile
	  stage.pc )

dnl should use AC_MSG_NOTICE(), but that's apparently not defined in
dnl autoconf 2.13.
AC_MSG_RESULT([])
AC_MSG_RESULT([*************************************************************])
AC_MSG_RESULT([])
AC_MSG_RESULT([Stage will be built for a $host system, with the following tools and options:])
AC_MSG_RESULT([  C++ compiler: $CXX $CXXFLAGS $PIXBUF_CFLAGS $FLTK_CFLAGS ])
AC_MSG_RESULT([])

if test $have_player = yes; then
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
