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
 * Desc: Program Entry point
 * Author: Andrew Howard, Richard Vaughan
 * Date: 12 Mar 2001
 * CVS: $Id: main.cc,v 1.61.2.2 2003-02-01 02:14:30 rtv Exp $
 */

#if HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdlib.h>
#include <assert.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h> /* for exit(2) */

#include "server.h"

///////////////////////////////////////////////////////////////////////////
// Global vars

// Quit signal
int quit = 0;
int paused = 0;

// SIGUSR1 toggles pause
void CatchSigUsr1( int signo )
{
  /*
    if( world )
    {
    world->m_enable = !world->m_enable;
    world->m_enable ? puts( "\nCLOCK STARTED" ) : puts( "\nCLOCK STOPPED" );
    }
    else */
  
  puts( "PAUSE FAILED - NO WORLD" );
 
}

///////////////////////////////////////////////////////////////////////////
// Print the usage string
void PrintUsage( void )
{
  printf("\nUsage: stage [options] <worldfile>\n"
	 "Options: <argument> [default]\n"
	 " -h\t\tPrint this message\n" 
	 " -g\t\tDo not start the X11 GUI\n"
	 " -n \t\tDo not start Player\n"
	 " -o\t\tEnable console status output\n"
	 " -v <float>\tSet the simulated time increment per cycle [0.1sec].\n"
	 " -u <float>\tSet the desired real time per cycle [0.1 sec].\n"
	 " -f \t\tRun as fast as possible; don't try to match real time\n"
	 " -s\t\tStart stage with the clock stopped (send SIGUSR1 to toggle clock)\n"
	 "\nSwitches for experimental/undocumented features:\n"
	 " -p <portnum>\tSet the server port [6601]\n"
	 " -c <hostname>\tRun as a client to a Stage server on hostname\n"
	 " -cl\t\tRun as a client to a Stage server on localhost\n"
	 " -l <filename>\tLog some timing and throughput statistics into <filename>.<incremental suffix>\n"
	 "\nCommand-line options override any configuration file equivalents.\n"
	 "See the Stage manual for details.\n"
	 "\nPart of the Player/Stage Project [http://playerstage.sourceforge.net].\n"
	 "Copyright 2000-2003 Richard Vaughan, Andrew Howard, Brian Gerkey and contributors\n"
	 "Released under the GNU General Public License"
	 " [http://www.gnu.org/copyleft/gpl.html].\n"
	 "\n"
	 );
}

///////////////////////////////////////////////////////////////////////////
// Clean up and quit
void StageQuit( void )
{  
  puts( "\n** Stage quitting **" );
 
  exit( 0 );
}

///////////////////////////////////////////////////////////////////////////
// Handle quit signals
void sig_quit(int signum)
{
  //PRINT_DEBUG1( "SIGNAL %d\n", signum );
  quit = 1;
}


///////////////////////////////////////////////////////////////////////////
// Program entry
//
int main(int argc, char **argv)
{  
  // hello world
  printf("\n** Stage  v%s ** ", (char*) VERSION);

  fflush( stdout );

  if( InitServer( argc, argv ) == -1 )
    {
      printf( "\n Server failed to initialize. Quitting." );
      quit = 1;
    }
  
  puts( "" ); // end the startup output line
  
  // Register callback for quit (^C,^\) events
  // - sig_quit function raises the quit flag 
  //signal(SIGINT, sig_quit );
  //signal(SIGQUIT, sig_quit );
  //signal(SIGTERM, sig_quit );
  //signal(SIGHUP, sig_quit );

  // catch clock start/stop commands
  //signal(SIGUSR1, CatchSigUsr1 );
  
  // the main loop  
  // update the simulation - stop when the quit flag is raised
  while( !quit ) 
    {
      
      // set up new clients
      if( AcceptConnections() == -1 ) break;
      
      // receive commands, property changes and subscriptions from
      // each client. will block until something is read.  if the
      // server receives 'update' commands from all clients, it'll
      // update the world
      if( ServiceConnections() == -1 ) break;

      // write out any changed, subscribed properties
      //server->WriteToClients();
    }
  
  // clean up and exit
  StageQuit();

  return 0; 
}


