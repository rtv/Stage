
dnl Manually inserted M4 code for building with OpenGL (and dependencies)
dnl which is provieded by the autoconf-archive package for many platforms,
dnl but does not seem to be available in MacPorts for OS X.
dnl Thanks to authors Braden McDaniel and Steven Johnson. - RTV 2008.02.24
dnl $Id: acinclude.m4,v 1.10 2008-02-24 20:13:57 rtv Exp $

dnl locally-defined macros go here

##### http://autoconf-archive.cryp.to/acx_pthread.html
#
# SYNOPSIS
#
#   ACX_PTHREAD([ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])
#
# DESCRIPTION
#
#   This macro figures out how to build C programs using POSIX threads.
#   It sets the PTHREAD_LIBS output variable to the threads library and
#   linker flags, and the PTHREAD_CFLAGS output variable to any special
#   C compiler flags that are needed. (The user can also force certain
#   compiler flags/libs to be tested by setting these environment
#   variables.)
#
#   Also sets PTHREAD_CC to any special C compiler that is needed for
#   multi-threaded programs (defaults to the value of CC otherwise).
#   (This is necessary on AIX to use the special cc_r compiler alias.)
#
#   NOTE: You are assumed to not only compile your program with these
#   flags, but also link it with them as well. e.g. you should link
#   with $PTHREAD_CC $CFLAGS $PTHREAD_CFLAGS $LDFLAGS ... $PTHREAD_LIBS
#   $LIBS
#
#   If you are only building threads programs, you may wish to use
#   these variables in your default LIBS, CFLAGS, and CC:
#
#          LIBS="$PTHREAD_LIBS $LIBS"
#          CFLAGS="$CFLAGS $PTHREAD_CFLAGS"
#          CC="$PTHREAD_CC"
#
#   In addition, if the PTHREAD_CREATE_JOINABLE thread-attribute
#   constant has a nonstandard name, defines PTHREAD_CREATE_JOINABLE to
#   that name (e.g. PTHREAD_CREATE_UNDETACHED on AIX).
#
#   ACTION-IF-FOUND is a list of shell commands to run if a threads
#   library is found, and ACTION-IF-NOT-FOUND is a list of commands to
#   run it if it is not found. If ACTION-IF-FOUND is not specified, the
#   default action will define HAVE_PTHREAD.
#
#   Please let the authors know if this macro fails on any platform, or
#   if you have any other suggestions or comments. This macro was based
#   on work by SGJ on autoconf scripts for FFTW (http://www.fftw.org/)
#   (with help from M. Frigo), as well as ac_pthread and hb_pthread
#   macros posted by Alejandro Forero Cuervo to the autoconf macro
#   repository. We are also grateful for the helpful feedback of
#   numerous users.
#
# LAST MODIFICATION
#
#   2007-07-29
#
# COPYLEFT
#
#   Copyright (c) 2007 Steven G. Johnson <stevenj@alum.mit.edu>
#
#   This program is free software: you can redistribute it and/or
#   modify it under the terms of the GNU General Public License as
#   published by the Free Software Foundation, either version 3 of the
#   License, or (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful, but
#   WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
#   General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program. If not, see
#   <http://www.gnu.org/licenses/>.
#
#   As a special exception, the respective Autoconf Macro's copyright
#   owner gives unlimited permission to copy, distribute and modify the
#   configure scripts that are the output of Autoconf when processing
#   the Macro. You need not follow the terms of the GNU General Public
#   License when using or distributing such scripts, even though
#   portions of the text of the Macro appear in them. The GNU General
#   Public License (GPL) does govern all other use of the material that
#   constitutes the Autoconf Macro.
#
#   This special exception to the GPL applies to versions of the
#   Autoconf Macro released by the Autoconf Macro Archive. When you
#   make and distribute a modified version of the Autoconf Macro, you
#   may extend this special exception to the GPL to apply to your
#   modified version as well.

AC_DEFUN([ACX_PTHREAD], [
AC_REQUIRE([AC_CANONICAL_HOST])
AC_LANG_SAVE
AC_LANG_C
acx_pthread_ok=no

# We used to check for pthread.h first, but this fails if pthread.h
# requires special compiler flags (e.g. on True64 or Sequent).
# It gets checked for in the link test anyway.

# First of all, check if the user has set any of the PTHREAD_LIBS,
# etcetera environment variables, and if threads linking works using
# them:
if test x"$PTHREAD_LIBS$PTHREAD_CFLAGS" != x; then
        save_CFLAGS="$CFLAGS"
        CFLAGS="$CFLAGS $PTHREAD_CFLAGS"
        save_LIBS="$LIBS"
        LIBS="$PTHREAD_LIBS $LIBS"
        AC_MSG_CHECKING([for pthread_join in LIBS=$PTHREAD_LIBS with CFLAGS=$PTHREAD_CFLAGS])
        AC_TRY_LINK_FUNC(pthread_join, acx_pthread_ok=yes)
        AC_MSG_RESULT($acx_pthread_ok)
        if test x"$acx_pthread_ok" = xno; then
                PTHREAD_LIBS=""
                PTHREAD_CFLAGS=""
        fi
        LIBS="$save_LIBS"
        CFLAGS="$save_CFLAGS"
fi

# We must check for the threads library under a number of different
# names; the ordering is very important because some systems
# (e.g. DEC) have both -lpthread and -lpthreads, where one of the
# libraries is broken (non-POSIX).

# Create a list of thread flags to try.  Items starting with a "-" are
# C compiler flags, and other items are library names, except for "none"
# which indicates that we try without any flags at all, and "pthread-config"
# which is a program returning the flags for the Pth emulation library.

acx_pthread_flags="pthreads none -Kthread -kthread lthread -pthread -pthreads -mthreads pthread --thread-safe -mt pthread-config"

# The ordering *is* (sometimes) important.  Some notes on the
# individual items follow:

# pthreads: AIX (must check this before -lpthread)
# none: in case threads are in libc; should be tried before -Kthread and
#       other compiler flags to prevent continual compiler warnings
# -Kthread: Sequent (threads in libc, but -Kthread needed for pthread.h)
# -kthread: FreeBSD kernel threads (preferred to -pthread since SMP-able)
# lthread: LinuxThreads port on FreeBSD (also preferred to -pthread)
# -pthread: Linux/gcc (kernel threads), BSD/gcc (userland threads)
# -pthreads: Solaris/gcc
# -mthreads: Mingw32/gcc, Lynx/gcc
# -mt: Sun Workshop C (may only link SunOS threads [-lthread], but it
#      doesn't hurt to check since this sometimes defines pthreads too;
#      also defines -D_REENTRANT)
#      ... -mt is also the pthreads flag for HP/aCC
# pthread: Linux, etcetera
# --thread-safe: KAI C++
# pthread-config: use pthread-config program (for GNU Pth library)

case "${host_cpu}-${host_os}" in
        *solaris*)

        # On Solaris (at least, for some versions), libc contains stubbed
        # (non-functional) versions of the pthreads routines, so link-based
        # tests will erroneously succeed.  (We need to link with -pthreads/-mt/
        # -lpthread.)  (The stubs are missing pthread_cleanup_push, or rather
        # a function called by this macro, so we could check for that, but
        # who knows whether they'll stub that too in a future libc.)  So,
        # we'll just look for -pthreads and -lpthread first:

        acx_pthread_flags="-pthreads pthread -mt -pthread $acx_pthread_flags"
        ;;
esac

if test x"$acx_pthread_ok" = xno; then
for flag in $acx_pthread_flags; do

        case $flag in
                none)
                AC_MSG_CHECKING([whether pthreads work without any flags])
                ;;

                -*)
                AC_MSG_CHECKING([whether pthreads work with $flag])
                PTHREAD_CFLAGS="$flag"
                ;;

		pthread-config)
		AC_CHECK_PROG(acx_pthread_config, pthread-config, yes, no)
		if test x"$acx_pthread_config" = xno; then continue; fi
		PTHREAD_CFLAGS="`pthread-config --cflags`"
		PTHREAD_LIBS="`pthread-config --ldflags` `pthread-config --libs`"
		;;

                *)
                AC_MSG_CHECKING([for the pthreads library -l$flag])
                PTHREAD_LIBS="-l$flag"
                ;;
        esac

        save_LIBS="$LIBS"
        save_CFLAGS="$CFLAGS"
        LIBS="$PTHREAD_LIBS $LIBS"
        CFLAGS="$CFLAGS $PTHREAD_CFLAGS"

        # Check for various functions.  We must include pthread.h,
        # since some functions may be macros.  (On the Sequent, we
        # need a special flag -Kthread to make this header compile.)
        # We check for pthread_join because it is in -lpthread on IRIX
        # while pthread_create is in libc.  We check for pthread_attr_init
        # due to DEC craziness with -lpthreads.  We check for
        # pthread_cleanup_push because it is one of the few pthread
        # functions on Solaris that doesn't have a non-functional libc stub.
        # We try pthread_create on general principles.
        AC_TRY_LINK([#include <pthread.h>],
                    [pthread_t th; pthread_join(th, 0);
                     pthread_attr_init(0); pthread_cleanup_push(0, 0);
                     pthread_create(0,0,0,0); pthread_cleanup_pop(0); ],
                    [acx_pthread_ok=yes])

        LIBS="$save_LIBS"
        CFLAGS="$save_CFLAGS"

        AC_MSG_RESULT($acx_pthread_ok)
        if test "x$acx_pthread_ok" = xyes; then
                break;
        fi

        PTHREAD_LIBS=""
        PTHREAD_CFLAGS=""
done
fi

# Various other checks:
if test "x$acx_pthread_ok" = xyes; then
        save_LIBS="$LIBS"
        LIBS="$PTHREAD_LIBS $LIBS"
        save_CFLAGS="$CFLAGS"
        CFLAGS="$CFLAGS $PTHREAD_CFLAGS"

        # Detect AIX lossage: JOINABLE attribute is called UNDETACHED.
	AC_MSG_CHECKING([for joinable pthread attribute])
	attr_name=unknown
	for attr in PTHREAD_CREATE_JOINABLE PTHREAD_CREATE_UNDETACHED; do
	    AC_TRY_LINK([#include <pthread.h>], [int attr=$attr; return attr;],
                        [attr_name=$attr; break])
	done
        AC_MSG_RESULT($attr_name)
        if test "$attr_name" != PTHREAD_CREATE_JOINABLE; then
            AC_DEFINE_UNQUOTED(PTHREAD_CREATE_JOINABLE, $attr_name,
                               [Define to necessary symbol if this constant
                                uses a non-standard name on your system.])
        fi

        AC_MSG_CHECKING([if more special flags are required for pthreads])
        flag=no
        case "${host_cpu}-${host_os}" in
            *-aix* | *-freebsd* | *-darwin*) flag="-D_THREAD_SAFE";;
            *solaris* | *-osf* | *-hpux*) flag="-D_REENTRANT";;
        esac
        AC_MSG_RESULT(${flag})
        if test "x$flag" != xno; then
            PTHREAD_CFLAGS="$flag $PTHREAD_CFLAGS"
        fi

        LIBS="$save_LIBS"
        CFLAGS="$save_CFLAGS"

        # More AIX lossage: must compile with xlc_r or cc_r
	if test x"$GCC" != xyes; then
          AC_CHECK_PROGS(PTHREAD_CC, xlc_r cc_r, ${CC})
        else
          PTHREAD_CC=$CC
	fi
else
        PTHREAD_CC="$CC"
fi

AC_SUBST(PTHREAD_LIBS)
AC_SUBST(PTHREAD_CFLAGS)
AC_SUBST(PTHREAD_CC)

# Finally, execute ACTION-IF-FOUND/ACTION-IF-NOT-FOUND:
if test x"$acx_pthread_ok" = xyes; then
        ifelse([$1],,AC_DEFINE(HAVE_PTHREAD,1,[Define if you have POSIX threads libraries and header files.]),[$1])
        :
else
        acx_pthread_ok=no
        $2
fi
AC_LANG_RESTORE
])dnl ACX_PTHREAD


dnl
dnl Check whether the compiler for the current language is Microsoft.
dnl
dnl This macro is modeled after _AC_LANG_COMPILER_GNU in the GNU Autoconf
dnl implementation.
dnl
dnl version: 1.0
dnl author: Braden McDaniel <braden@endoframe.com>
dnl
dnl This program is free software; you can redistribute it and/or modify
dnl it under the terms of the GNU General Public License as published by
dnl the Free Software Foundation; either version 2, or (at your option)
dnl any later version.
dnl
dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU General Public License for more details.
dnl
dnl You should have received a copy of the GNU General Public License
dnl along with this program; if not, write to the Free Software
dnl Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
dnl 02110-1301, USA.
dnl
dnl As a special exception, the you may copy, distribute and modify the
dnl configure scripts that are the output of Autoconf when processing
dnl the Macro.  You need not follow the terms of the GNU General Public
dnl License when using or distributing such scripts.
dnl
AC_DEFUN([AX_LANG_COMPILER_MS],
[AC_CACHE_CHECK([whether we are using the Microsoft _AC_LANG compiler],
                [ax_cv_[]_AC_LANG_ABBREV[]_compiler_ms],
[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([], [[#ifndef _MSC_VER
       choke me
#endif
]])],
                   [ax_compiler_ms=yes],
                   [ax_compiler_ms=no])
ax_cv_[]_AC_LANG_ABBREV[]_compiler_ms=$ax_compiler_ms
])])


dnl
dnl AX_CHECK_GL
dnl
dnl Check for an OpenGL implementation.  If GL is found, the required compiler
dnl and linker flags are included in the output variables "GL_CFLAGS" and
dnl "GL_LIBS", respectively.  If no usable GL implementation is found, "no_gl"
dnl is set to "yes".
dnl
dnl If the header "GL/gl.h" is found, "HAVE_GL_GL_H" is defined.  If the header
dnl "OpenGL/gl.h" is found, HAVE_OPENGL_GL_H is defined.  These preprocessor
dnl definitions may not be mutually exclusive.
dnl
dnl version: 2.0
dnl author: Braden McDaniel <braden@endoframe.com>
dnl
dnl This program is free software; you can redistribute it and/or modify
dnl it under the terms of the GNU General Public License as published by
dnl the Free Software Foundation; either version 2, or (at your option)
dnl any later version.
dnl
dnl This program is distributed in the dhope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU General Public License for more details.
dnl
dnl You should have received a copy of the GNU General Public License
dnl along with this program; if not, write to the Free Software
dnl Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
dnl 02110-1301, USA.
dnl
dnl As a special exception, the you may copy, distribute and modify the
dnl configure scripts that are the output of Autoconf when processing
dnl the Macro.  You need not follow the terms of the GNU General Public
dnl License when using or distributing such scripts.
dnl
AC_DEFUN([AX_CHECK_GL],
[AC_REQUIRE([AC_PATH_X])dnl
AC_REQUIRE([ACX_PTHREAD])dnl

AC_LANG_PUSH([C])
AX_LANG_COMPILER_MS
AS_IF([test X$ax_compiler_ms = Xno],
      [GL_CFLAGS="${PTHREAD_CFLAGS}"; GL_LIBS="${PTHREAD_LIBS} -lm"])

#
# Use x_includes and x_libraries if they have been set (presumably by
# AC_PATH_X).
#
AS_IF([test "X$no_x" != "Xyes"],
      [AS_IF([test -n "$x_includes"],
             [GL_CFLAGS="-I${x_includes} ${GL_CFLAGS}"])]
       AS_IF([test -n "$x_libraries"],
             [GL_LIBS="-L${x_libraries} -lX11 ${GL_LIBS}"]))

ax_save_CPPFLAGS="${CPPFLAGS}"
CPPFLAGS="${GL_CFLAGS} ${CPPFLAGS}"
AC_CHECK_HEADERS([GL/gl.h OpenGL/gl.h])
CPPFLAGS="${ax_save_CPPFLAGS}"

AC_CHECK_HEADERS([windows.h])

m4_define([AX_CHECK_GL_PROGRAM],
          [AC_LANG_PROGRAM([[
# if defined(HAVE_WINDOWS_H) && defined(_WIN32)
#   include <windows.h>
# endif
# ifdef HAVE_GL_GL_H
#   include <GL/gl.h>
# elif defined(HAVE_OPENGL_GL_H)
#   include <OpenGL/gl.h>
# else
#   error no gl.h
# endif]],
                           [[glBegin(0)]])])

AC_CACHE_CHECK([for OpenGL library], [ax_cv_check_gl_libgl],
[ax_cv_check_gl_libgl="no"
ax_save_CPPFLAGS="${CPPFLAGS}"
CPPFLAGS="${GL_CFLAGS} ${CPPFLAGS}"
ax_save_LIBS="${LIBS}"
LIBS=""
ax_check_libs="-lopengl32 -lGL"
for ax_lib in ${ax_check_libs}; do
  AS_IF([test X$ax_compiler_ms = Xyes],
        [ax_try_lib=`echo $ax_lib | sed -e 's/^-l//' -e 's/$/.lib/'`],
        [ax_try_lib="${ax_lib}"])
  LIBS="${ax_try_lib} ${GL_LIBS} ${ax_save_LIBS}"
  AC_LINK_IFELSE(
[AX_CHECK_GL_PROGRAM],
[ax_cv_check_gl_libgl="${ax_try_lib}"; break],
[ax_check_gl_dylib_flag='-dylib_file /System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib:/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib'
LIBS="${ax_try_lib} ${ax_check_gl_dylib_flag} ${GL_LIBS} ${ax_save_LIBS}"
AC_LINK_IFELSE([AX_CHECK_GL_PROGRAM],
               [ax_cv_check_gl_libgl="${ax_try_lib} ${ax_check_gl_dylib_flag}"; break])])
done

AS_IF([test "X$ax_cv_check_gl_libgl" = Xno -a "X$no_x" = Xyes],
[LIBS='-framework OpenGL'
AC_LINK_IFELSE([AX_CHECK_GL_PROGRAM],
               [ax_cv_check_gl_libgl="$LIBS"])])

LIBS=${ax_save_LIBS}
CPPFLAGS=${ax_save_CPPFLAGS}])

AS_IF([test "X$ax_cv_check_gl_libgl" = Xno],
      [no_gl=yes; GL_CFLAGS=""; GL_LIBS=""],
      [GL_LIBS="${ax_cv_check_gl_libgl} ${GL_LIBS}"])
AC_LANG_POP([C])

AC_SUBST([GL_CFLAGS])
AC_SUBST([GL_LIBS])
])dnl


dnl
dnl AX_CHECK_GLU
dnl
dnl Check for GLU.  If GLU is found, the required preprocessor and linker flags
dnl are included in the output variables "GLU_CFLAGS" and "GLU_LIBS",
dnl respectively.  If no GLU implementation is found, "no_glu" is set to "yes".
dnl
dnl If the header "GL/glu.h" is found, "HAVE_GL_GLU_H" is defined.  If the
dnl header "OpenGL/glu.h" is found, HAVE_OPENGL_GLU_H is defined.  These
dnl preprocessor definitions may not be mutually exclusive.
dnl
dnl Some implementations (in particular, some versions of Mac OS X) are known
dnl to treat the GLU tesselator callback function type as "GLvoid (*)(...)"
dnl rather than the standard "GLvoid (*)()".  If the former condition is
dnl detected, this macro defines "HAVE_VARARGS_GLU_TESSCB".
dnl
dnl version: 2.1
dnl author: Braden McDaniel <braden@endoframe.com>
dnl
dnl This program is free software; you can redistribute it and/or modify
dnl it under the terms of the GNU General Public License as published by
dnl the Free Software Foundation; either version 2, or (at your option)
dnl any later version.
dnl
dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU General Public License for more details.
dnl
dnl You should have received a copy of the GNU General Public License
dnl along with this program; if not, write to the Free Software
dnl Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
dnl 02110-1301, USA.
dnl
dnl As a special exception, the you may copy, distribute and modify the
dnl configure scripts that are the output of Autoconf when processing
dnl the Macro.  You need not follow the terms of the GNU General Public
dnl License when using or distributing such scripts.
dnl
AC_DEFUN([AX_CHECK_GLU],
[AC_REQUIRE([AX_CHECK_GL])dnl
AC_REQUIRE([AC_PROG_CXX])dnl
GLU_CFLAGS="${GL_CFLAGS}"

ax_save_CPPFLAGS="${CPPFLAGS}"
CPPFLAGS="${GL_CFLAGS} ${CPPFLAGS}"
AC_CHECK_HEADERS([GL/glu.h OpenGL/glu.h])
CPPFLAGS="${ax_save_CPPFLAGS}"

m4_define([AX_CHECK_GLU_PROGRAM],
          [AC_LANG_PROGRAM([[
# if defined(HAVE_WINDOWS_H) && defined(_WIN32)
#   include <windows.h>
# endif
# ifdef HAVE_GL_GLU_H
#   include <GL/glu.h>
# elif defined(HAVE_OPENGL_GLU_H)
#   include <OpenGL/glu.h>
# else
#   error no glu.h
# endif]],
                           [[gluBeginCurve(0)]])])

AC_CACHE_CHECK([for OpenGL Utility library], [ax_cv_check_glu_libglu],
[ax_cv_check_glu_libglu="no"
ax_save_CPPFLAGS="${CPPFLAGS}"
CPPFLAGS="${GL_CFLAGS} ${CPPFLAGS}"
ax_save_LIBS="${LIBS}"

#
# First, check for the possibility that everything we need is already in
# GL_LIBS.
#
LIBS="${GL_LIBS} ${ax_save_LIBS}"
#
# libGLU typically links with libstdc++ on POSIX platforms.
# However, setting the language to C++ means that test program
# source is named "conftest.cc"; and Microsoft cl doesn't know what
# to do with such a file.
#
AC_LANG_PUSH([C++])
AS_IF([test X$ax_compiler_ms = Xyes],
      [AC_LANG_PUSH([C])])
AC_LINK_IFELSE(
[AX_CHECK_GLU_PROGRAM],
[ax_cv_check_glu_libglu=yes],
[LIBS=""
ax_check_libs="-lglu32 -lGLU"
for ax_lib in ${ax_check_libs}; do
  AS_IF([test X$ax_compiler_ms = Xyes],
        [ax_try_lib=`echo $ax_lib | sed -e 's/^-l//' -e 's/$/.lib/'`],
        [ax_try_lib="${ax_lib}"])
  LIBS="${ax_try_lib} ${GL_LIBS} ${ax_save_LIBS}"
  AC_LINK_IFELSE([AX_CHECK_GLU_PROGRAM],
                 [ax_cv_check_glu_libglu="${ax_try_lib}"; break])
done
])
AS_IF([test X$ax_compiler_ms = Xyes],
      [AC_LANG_POP([C])])
AC_LANG_POP([C++])

LIBS=${ax_save_LIBS}
CPPFLAGS=${ax_save_CPPFLAGS}])
AS_IF([test "X$ax_cv_check_glu_libglu" = Xno],
      [no_glu=yes; GLU_CFLAGS=""; GLU_LIBS=""],
      [AS_IF([test "X$ax_cv_check_glu_libglu" = Xyes],
             [GLU_LIBS="$GL_LIBS"],
             [GLU_LIBS="${ax_cv_check_glu_libglu} ${GL_LIBS}"])])
AC_SUBST([GLU_CFLAGS])
AC_SUBST([GLU_LIBS])

#
# Some versions of Mac OS X include a broken interpretation of the GLU
# tesselation callback function signature.
#
AS_IF([test "X$ax_cv_check_glu_libglu" != Xno],
[AC_CACHE_CHECK([for varargs GLU tesselator callback function type],
                [ax_cv_varargs_glu_tesscb],
[ax_cv_varargs_glu_tesscb=no
ax_save_CFLAGS="$CFLAGS"
CFLAGS="$GL_CFLAGS $CFLAGS"
AC_COMPILE_IFELSE(
[AC_LANG_PROGRAM([[
# ifdef HAVE_GL_GLU_H
#   include <GL/glu.h>
# else
#   include <OpenGL/glu.h>
# endif]],
                 [[GLvoid (*func)(...); gluTessCallback(0, 0, func)]])],
[ax_cv_varargs_glu_tesscb=yes])
CFLAGS="$ax_save_CFLAGS"])
AS_IF([test X$ax_cv_varargs_glu_tesscb = Xyes],
      [AC_DEFINE([HAVE_VARARGS_GLU_TESSCB], [1],
                 [Use nonstandard varargs form for the GLU tesselator callback])])])
])
