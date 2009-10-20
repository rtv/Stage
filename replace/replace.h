/*  $Id: replace.h,v 1.3 2008-01-15 01:25:42 rtv Exp $
 *
 * replacement function prototypes
 */

#ifndef _REPLACE_H
#define _REPLACE_H


#if HAVE_CONFIG_H
  #include <config.h>
#endif

/* Compatibility definitions for System V `poll' interface.
   Copyright (C) 1994,96,97,98,99,2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

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

#ifdef __cplusplus
extern "C" {
#endif

#if !HAVE_POLL
/* Event types that can be polled for.  These bits may be set in `events'
   to indicate the interesting event types; they will appear in `revents'
   to indicate the status of the file descriptor.  */
#define POLLIN          01              /* There is data to read.  */
#define POLLPRI         02              /* There is urgent data to read.  */
#define POLLOUT         04              /* Writing now will not block.  */

/* Some aliases.  */
#define POLLWRNORM      POLLOUT
#define POLLRDNORM      POLLIN
#define POLLRDBAND      POLLPRI

/* Event types always implicitly polled for.  These bits need not be set in
   `events', but they will appear in `revents' to indicate the status of
   the file descriptor.  */
#define POLLERR         010             /* Error condition.  */
#define POLLHUP         020             /* Hung up.  */
#define POLLNVAL        040             /* Invalid polling request.  */

/* Canonical number of polling requests to read in at a time in poll.  */
#define NPOLLFILE       30

/* Data structure describing a polling request.  */
struct pollfd
  {
    int fd;			/* File descriptor to poll.  */
    short int events;		/* Types of events poller cares about.  */
    short int revents;		/* Types of events that actually occurred.  */
  };


/* Poll the file descriptors described by the NFDS structures starting at
   FDS.  If TIMEOUT is nonzero and not -1, allow TIMEOUT milliseconds for
   an event to occur; if TIMEOUT is -1, block until an event occurs.
   Returns the number of file descriptors with events, zero if timed out,
   or -1 for errors.  */
int poll (struct pollfd *fds, unsigned long int nfds, int timeout);
#else
#include <sys/poll.h>  /* for poll(2) */
#endif // !HAVE_POLL

  //#if !HAVE_SCANDIR
  //#include <sys/types.h>
  //#include <dirent.h>
  //int scandir(const char *dir, struct dirent ***namelist,
  //        int (*select)(const struct dirent *),
  //        int (*compar)(const struct dirent **, const struct dirent **));
  //#endif //!HAVE_SCANDIR

  /*

#if !HAVE_DIRNAME
  char * dirname (char *path);
#else
  #include <libgen.h> // for dirname(3)
#endif // !HAVE_DIRNAME

#if !HAVE_BASENAME
  const char * basename (const char* filename);
#else
  #include <libgen.h> // for basename(3)
#endif // !HAVE_BASENAME
  */

#ifdef __cplusplus
}
#endif

#endif

