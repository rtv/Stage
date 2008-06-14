/* dirname - return directory part of PATH.
   Copyright (C) 1996 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1996.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/*
 * $Id: dirname.c,v 1.1 2002-12-05 01:50:22 rtv Exp $
 *
 * taken from glibc, modified slightly for standalone compilation, and used as
 * a fallback implementation when dirname() is not available. - BPG
 */

#include <string.h>

char *
dirname (char *path)
{
  static const char dot[] = ".";
  char *last_slash;

  /* Find last '/'.  */
  last_slash = path != NULL ? strrchr (path, '/') : NULL;

  if (last_slash == path)
    /* The last slash is the first character in the string.  We have to
       return "/".  */
    ++last_slash;
  else if (last_slash != NULL && last_slash[1] == '\0')
    /* The '/' is the last character, we have to look further.  */
    last_slash = memchr (path, last_slash - path, '/');

  if (last_slash != NULL)
    /* Terminate the path.  */
    last_slash[0] = '\0';
  else
    /* This assignment is ill-designed but the XPG specs require to
       return a string containing "." in any case no directory part is
       found and so a static and constant string is required.  */
    path = (char *) dot;

  return path;
}

