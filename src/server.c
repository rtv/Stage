/*
 *  Stage : a multi-robot simulator.
 *  Copyright (C) 2001, 2002 Richard Vaughan, Andrew Howard and Brian Gerkey.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
/*
 * Desc: This class implements the server, or main, instance of Stage.
 * Author: Richard Vaughan, Andrew Howard
 * Date: 6 Jun 2002
 * CVS info: $Id: server.c,v 1.1.2.5 2003-02-03 03:07:26 rtv Exp $
 */
#if HAVE_CONFIG_H
  #include <config.h>
#endif
#if HAVE_STRINGS_H
  #include <strings.h>
#endif

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

#include "replace.h"

#include "stageio.h"
#include "cwrapper.h"

typedef	struct sockaddr SA; // useful abbreviation
 
#define DEBUG
//#define VERBOSE


//char* hostname = "localhost";

// dummy timer signal func
void TimerHandler( int val )
{
#ifdef VERBOSE
  PRINT_DEBUG( "TIMER HANDLER" );
#endif

  //g_timer_events++;

  // re-install signal handler for timing
  assert( signal( SIGALRM, &TimerHandler ) != SIG_ERR );

  PRINT_DEBUG( "timer expired" );
}  




