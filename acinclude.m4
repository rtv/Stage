define([RTK2_TESTS],
[
dnl we need GTK to build RTK-based GUIs (e.g., playerv)
AC_CHECK_PROG([have_gtk], [gtk-config], [yes], [no])
AM_CONDITIONAL(HAVE_GTK, test x$have_gtk = xyes)

dnl RTK2 uses libjpeg to export images.
AC_CHECK_HEADER(jpeglib.h,
  AC_DEFINE(HAVE_JPEGLIB_H,1,[include jpeg support])
  LIBJPEG="-ljpeg",,)
AC_SUBST(LIBJPEG)

AC_ARG_WITH(rtk,    [  --without-rtk           Don't build RTK-based the GUI(s)],,
with_rtk=yes)
if test "x$with_rtk" = "xyes"; then
  AC_DEFINE(INCLUDE_RTK2,1,[include the RTK GUI])
  GUI_DIR=rtkstage
  GUI_LIB=librtkstage.a
  GUI_LINK="-L../rtk2 -Lrtkstage -lrtkstage -lrtk $LIBJPEG `gtk-config --libs gtk gthread`"
fi
AM_CONDITIONAL(WITH_RTK_GUI, test x$with_rtk = xyes)])

