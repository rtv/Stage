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
 * CVS: $Id: main.cc,v 1.61.2.7 2003-02-04 03:35:38 rtv Exp $
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

#include "stageio.h"
#include "entity.hh"
#include "root.hh"

/*
#include "models/bitmap.hh"
#include "models/box.hh"
#include "models/bumperdevice.hh"
#include "models/broadcastdevice.hh"
#include "models/gpsdevice.hh"
#include "models/gripperdevice.hh"
#include "models/idardevice.hh"
#include "models/idarturretdevice.hh"
#include "models/fiducialfinderdevice.hh"
#include "models/laserdevice.hh"
#include "models/motedevice.hh"
#include "models/positiondevice.hh"
#include "models/powerdevice.hh"
#include "models/ptzdevice.hh"
#include "models/puck.hh"
#include "models/sonardevice.hh"
#include "models/truthdevice.hh"
#include "models/visiondevice.hh"
#include "models/regularmcldevice.hh"
//#include "models/bpsdevice.hh"
*/


// this array defines the models that are available to Stage. New
// devices must be added here.

libitem_t library_items[] = { 
  { "root", "black", (CFP)CRootDevice::Creator},
  { "box", "blue", (CFP)CEntity::Creator},
  /*
  { "bitmap", "black", (CFP)CBitmap::Creator},
  { "laser", "blue", (CFP)CLaserDevice::Creator},
  { "position", "red", (CFP)CPositionDevice::Creator},
  { "sonar", "green", (CFP)CSonarDevice::Creator},
  { "box", "yellow", (CFP)CBox::Creator},
  { "gps", "gray", (CFP)CGpsDevice::Creator},
  { "gripper", "blue", (CFP)CGripperDevice::Creator},
  { "idar", "DarkRed", (CFP)CIdarDevice::Creator},
  { "idarturret", "DarkRed", (CFP)CIdarTurretDevice::Creator},
  { "lbd", "gray", (CFP)CFiducialFinder::Creator},
  { "fiducialfinder", "gray", (CFP)CFiducialFinder::Creator},
  { "mote", "orange", (CFP)CMoteDevice::Creator},
  { "power", "wheat", (CFP)CPowerDevice::Creator},
  { "ptz", "magenta", (CFP)CPtzDevice::Creator},
  { "puck", "green", (CFP)CPuck::Creator},
  { "truth", "purple", (CFP)CTruthDevice::Creator},
  { "vision", "gray", (CFP)CVisionDevice::Creator},
  { "blobfinder", "gray", (CFP)CVisionDevice::Creator},
  { "broadcast", "brown", (CFP)CBroadcastDevice::Creator},
  { "bumper", "LightBlue", (CFP)CBumperDevice::Creator},
  { "regularmcl", "blue", (CFP)CRegularMCLDevice::Creator},
  // { "bps", BpsType, (CFP)CBpsDevice::Creator},
  */

  {NULL, NULL, NULL } // marks the end of the array
};  


// statically allocate a libray filled with the entries above
Library model_library( library_items );

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
  // reset the color of the console text and say 'bye.
  puts( "\n\033[00m** Stage quitting **" );
  exit( 0 );
}

///////////////////////////////////////////////////////////////////////////
// Handle quit signals
void sig_quit(int signum)
{
  //PRINT_DEBUG1( "SIGNAL %d\n", signum );
  quit = 1;
}

int HandleModel(  int connection, stage_model_t* model )
{
  PRINT_DEBUG1( "Received model on connection %d", connection );

  // create an entity. return success on getting a valid pointer
  return( model_library.CreateEntity( model ) ? 0 : -1);
}

int HandleProperty( int connection, stage_property_t* prop )
{
  PRINT_DEBUG1( "Received property on connection %d", connection );

  // get a pointer to the object with this id
  CEntity* ent = model_library.GetEntPtr( prop->id );
  
  // pass the property data to the entity to absorb
  ent->SetProperty( connection, // the connection number 
		    prop->property, // the property id
		    ((char*)prop)+sizeof(prop), // the raw data
		    prop->len ); // the length of the raw data

  return 0; //success
}

int HandleCommand(  int connection, stage_cmd_t* cmd )
{
  PRINT_DEBUG2( "Received command %d on connection %d", *cmd, connection );

  switch( *cmd ) // 
    {
    case STG_CMD_SAVE:
    case STG_CMD_LOAD:
    case STG_CMD_PAUSE:
    case STG_CMD_UNPAUSE:     
      PRINT_WARN1( "command %d not implemented\n", *cmd );
      break;
      
    default:
      PRINT_WARN1( "Unknown command (%d) not handled", *cmd);
      break;
    }

  return 0; //success
}

///////////////////////////////////////////////////////////////////////////
// Program entry
//
int main(int argc, char **argv)
{  
  // hello world
  printf("\n** Stage  v%s ** ", (char*) VERSION);

  fflush( stdout );

  if( SIOInitServer( argc, argv ) == -1 )
    {
      printf( "\n Server failed to initialize. Quitting." );
      quit = 1;
    }

  //GuiInit( argc, argv );
  
  puts( "" ); // end the startup output line
  
  // Register callback for quit (^C,^\) events
  // - sig_quit function raises the quit flag 
  //signal(SIGINT, sig_quit );
  //signal(SIGQUIT, sig_quit );
  //signal(SIGTERM, sig_quit );
  //signal(SIGHUP, sig_quit );

  // catch clock start/stop commands
  //signal(SIGUSR1, CatchSigUsr1 );
  
  int c = 0;
  // the main loop  
  // update the simulation - stop when the quit flag is raised
  while( !quit ) 
    {
      //printf( "cycle %d\n", c++ );

      // set up new clients
      if( SIOAcceptConnections() == -1 ) break;
      
      // receive commands, property changes and subscriptions from
      // each client. will block until something is read.  if the
      // server receives 'update' commands from all clients, it'll
      // update the world
      if( SIOServiceConnections( &HandleCommand, 
			      &HandleModel, 
			      &HandleProperty ) == -1 ) break;
      
      // update the simulation model
      if( CEntity::root ) CEntity::root->Update( CEntity::simtime+=0.1 );
     
      stage_property_t* props = NULL;
      size_t props_len = 0;
      //props = CEntity::root->GetChangedProperties()
      
      // pass the changed props to the server, which distributes them
      // on the appropriate connections
      if( SIOReportResults( CEntity::simtime, (char*)props, props_len) == -1 ) 
	break;
      
      sleep( 1 ); // just stop the powerbook getting hot :)
    }
  
  // clean up and exit
  StageQuit();

  return 0; 
}


