/*
 *  Stage : a multi-robot simulator.  
 *
 *  Copyright (C) 2001, 2002, 2003 Richard Vaughan, Andrew Howard and
 *  Brian Gerkey.
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
 * Author: Richard Vaughan, Andrew Howard
 * Date: 12 Mar 2001
 * CVS: $Id: main.cc,v 1.61.2.30 2003-02-23 08:01:36 rtv Exp $
 */

#if HAVE_CONFIG_H
#  include <config.h>
#endif

#undef DEBUG
#undef VERBOSE

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h> /* for exit(2) */
#include <math.h>

#include "sio.h"
#include "entity.hh"

#include "models/puck.hh"
#include "models/sonar.hh"
/*
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
#include "models/truthdevice.hh"
#include "models/visiondevice.hh"
#include "models/regularmcldevice.hh"
//#include "models/bpsdevice.hh"
*/

// MODEL INSTALLATION --------------------------------------------
//
// this array defines the models that are available to Stage. New
// devices must be added here.

stage_libitem_t library_items[] = { 
  { "box", "black", (CFP)CEntity::Creator},
  { "puck", "green", (CFP)CPuck::Creator},
  { "sonar", "red", (CFP)CSonarModel::Creator},
  /*
    { "bitmap", "blue", (CFP)CBitmap::Creator},
  { "laser", "blue", (CFP)CLaserDevice::Creator},
  { "position", "red", (CFP)CPositionDevice::Creator},
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

// GUI INSTALLATION -----------------------------------------------------
//
// Stage handles multiple GUI libraries. Add your library functions
// here and set the counter correctly

#include "rtkgui/rtkgui.hh"


#ifdef INCLUDE_RTK2
const int gui_count = 1; // must match the number of entries in the array
stage_gui_library_item_t gui_library[] = {
  { "rtk", // install callback functions for the RTK2-based GUI 
    RtkGuiInit, RtkGuiLoad, RtkGuiUpdate, 
    RtkGuiEntityStartup, RtkGuiEntityShutdown,
    RtkGuiEntityPropertyChange
  }
};
#else
const int gui_count = 0; // must match the number of entries in the array
stage_gui_library_item_t gui_library[] = {};
#endif

///////////////////////////////////////////////////////////////////////////
// Global vars

double update_interval = 0.01; // seconds

// Quit signal
int quit = 0;
int paused = 0;

// catch SIGUSR1 to toggle pause
void CatchSigUsr1( int signo )
{
  paused = !paused; 
  paused ? PRINT_MSG("CLOCK STARTED" ) : PRINT_MSG( "CLOCK STOPPED" );
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
  PRINT_MSG( "closing all connections" );

  for( int c=0; c<SIOGetConnectionCount(); c++ )
    SIODestroyConnection( c );
  
  puts( "\033[00m** Stage quitting **" );
  exit( 0 );
}

///////////////////////////////////////////////////////////////////////////
// Handle quit signals
void sig_quit(int signum)
{
  PRINT_DEBUG1( "SIGNAL %d", signum );
  //quit = 1;
  StageQuit();
}

double TimevalSeconds( struct timeval *tv )
{
  return( (double)(tv->tv_sec) + (double)(tv->tv_usec) / (double)MILLION );
}

///////////////////////////////////////////////////////////////////////////
// Get the real time
// Returns time in sec since the first call of this function
double GetRealTime()
{
  static double start_time;
  static bool init = true;
  struct timeval tv;
 
  if( init )
    {
      gettimeofday( &tv, NULL );
      start_time = TimevalSeconds( &tv );
      init = false;
      PRINT_DEBUG1( "start time %.4f", start_time );
    }
  
  gettimeofday( &tv, NULL );
  return( TimevalSeconds( &tv ) - start_time );
}

int HandleLostConnection( int connection )
{
  // clears subscription data on this connection for all models
  if( CEntity::root )
    CEntity::root->DestroyConnection( connection );

  return 0; // success
}

int HandleModel(  int connection, void* data, size_t len, 
		  stage_buffer_t* replies )
{
  assert( len == sizeof(stage_model_t) );
  stage_model_t* model = (stage_model_t*)data;
  
  PRINT_DEBUG1( "Received model on connection %d", connection );
  
  // create an entity. returns a pointer to a CEntity, and sets the model.id
  if( model_library.CreateEntity( model ) == NULL )
    {
      PRINT_WARN( "failed to create an entity" );
      return -1;
    }
  
  // return the model packet to the client with the id filled in.
  SIOWriteMessage( connection, CEntity::simtime, 
		   STG_HDR_MODEL, 
		   (char*)model, sizeof(stage_model_t) );
  
  SIOWriteMessage( connection, CEntity::simtime, STG_HDR_CONTINUE, NULL, 0 );

  return 0; // success
}

int HandleProperty( int connection,  void* data, size_t len,
		    stage_buffer_t* replies )
{
  PRINT_DEBUG3( "Received %d byte property request "
		"(of which header is %d bytes ) on connection %d", 
		(int)len, (int)sizeof(stage_property_t), connection );
  
  assert( len >= sizeof(stage_property_t) );
  stage_property_t* prop = (stage_property_t*)data;
  
  PRINT_DEBUG2( "Received property %d on connection %d", 
		prop->property, connection );
  

  // get a pointer to the object with this id
  CEntity* ent = model_library.GetEntFromId( prop->id );

  if( !ent )
    PRINT_WARN2( "Received property %d for non-existent model %d", prop->id, prop->property );
  else
    {
      // pass the property data to the entity to absorb
      ent->Property( connection, // the connection number 
		     prop->property, // the property id
		     ((char*)data)+sizeof(stage_property_t), // the raw data
		     prop->len, // the length of the raw data      
		     replies ); // a buffer to store replies in
    }

  return 0; //success
}

int HandleCommand(  int connection, void* data, size_t len, 
		    stage_buffer_t* replies )
{
  assert( len == sizeof(stage_cmd_t) );
  stage_cmd_t* cmd = (stage_cmd_t*)data;
  
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

int HandleGui( int connection, void* data, size_t len ,
	       stage_buffer_t* replies )
{
  assert( len == sizeof(stage_gui_config_t) );
  stage_gui_config_t* cfg  = (stage_gui_config_t*)data;
  assert( cfg );

  PRINT_DEBUG1( "received GUI configuraton for library %s", 
		cfg->token );

  // find the library index of the GUI library with this token
  int l; 
  for( l=0; l < gui_count; l++ )
    if( strcmp( gui_library[l].token, cfg->token ) == 0 )
      // call the config function from the correct library and return
      return( (*(gui_library[l].load_func))( cfg ) );
  
  PRINT_WARN1( "Received config for unknown GUI library %s", cfg->token );
  
  return -1;// fail
}

int GuiInit( int argc, char** argv )
{
  int g;
  for( g=0; g<gui_count; g++ )
    {
      PRINT_DEBUG1( "init of gui %d", g );
      if( (*gui_library[g].init_func)(argc, argv) == -1 )
	{
	  PRINT_WARN1( "Gui initialization failed on GUI %d\n", g );
	  return -1;
	}
    }
  return 0; // success
}


int GuiEntityStartup( CEntity* ent )
{
  assert(ent);
  
  int g;
  for( g=0; g<gui_count; g++ )
    {
      PRINT_DEBUG2( "gui startup for ent %d on GUI %d", ent->stage_id, g );
      
      if( (*gui_library[g].createmodel_func)(ent) == -1 )
	{
	  PRINT_WARN1( "Gui create model failed on GUI %d", g );
	  return -1;
	}
      
      // start the children too!
      CEntity* ch;
      for( ch = ent->child_list; ch; ch = ch->next )
	if( GuiEntityStartup( ch ) == -1 )
	  return -1;
    }
  return 0;
}

int GuiUpdate( void )
{
  int g;
  for( g=0; g<gui_count; g++ )
    {
      //printf( "GUI Update on GUI %d\n", g );
      
       if( (*gui_library[g].update_func)() == -1 )
	{
	  PRINT_WARN1( "Gui update failed on GUI %d", g );
	  return -1;
	}
    }
  return 0;
}

int GuiEntityPropertyChange( CEntity* ent, stage_prop_id_t prop )
{
  assert(ent);
  
  int g;
  for( g=0; g<gui_count; g++ )
    {
      PRINT_DEBUG3( "gui prop change for ent %d on GUI %d prop %d", 
		    ent->stage_id, g, prop );
      
      if( (*gui_library[g].tweakmodel_func)(ent, prop) == -1 )
	{
	  PRINT_WARN1( "Gui property change failed on GUI %d", g );
	  return -1;
	}
    }
  return 0;
}

int GuiEntityShutdown( CEntity* ent )
{
  assert(ent);
  
  int g;
  for( g=0; g<gui_count; g++ )
    {
      PRINT_DEBUG2( "gui shutdown for ent %d on GUI %d", ent->stage_id, g );

      if( (*gui_library[g].destroymodel_func)(ent) == -1 )
	{
	  PRINT_WARN1( "Gui destroy model failed on GUI %d", g );
	  return -1;
	}      
    }

  return 0;
}

int PublishDirty()
{
  // TODO = fix this!
  char* propdata = (char*)new char[100000];
  
  for( int con=0; con < SIOGetConnectionCount(); con++ )
    {
      stage_buffer_t *dirty = SIOCreateBuffer();
      int dirty_prop_count = 0;
      
      for( int ent=0; ent< model_library.model_count; ent++ )
	{
	  if( CEntity *entp = model_library.GetEntFromId(ent) )
	    {	      
	      int p;
	      for( p=0; p<STG_PROPERTY_COUNT; p++ )
		if( entp->subscriptions[con][p].subscribed &&
		    entp->subscriptions[con][p].dirty )
		  {
		    stage_prop_id_t propid = (stage_prop_id_t)p;
		    
		    PRINT_WARN3( "ent: %d con: %d prop: %s needs exported",
				 ent, con, SIOPropString(propid) );
		    
		    // add the dirty property to the dirty buffer
		    assert( entp->Property( con, propid, NULL, 0, dirty ) 
			    != -1 );

		    entp->subscriptions[con][propid].dirty = 0;
		    dirty_prop_count++;
		  }
	    }
	}
      
      if( dirty_prop_count > 0 )
	{
	  PRINT_WARN2( "writing %d dirty properties on con %d", dirty_prop_count, con );
	  SIOWriteMessage( con, CEntity::simtime, 
			   STG_HDR_PROPS, dirty->data, dirty->len );
	}
      
      SIOFreeBuffer( dirty );
   }
  
  delete[] propdata;
		    
  return 0; // success
}



double TimespecSeconds( struct timespec *ts )
{
  return( (double)(ts->tv_sec) + (double)(ts->tv_nsec) / (double)BILLION);
}


void PackTimespec( struct timespec *ts, double seconds )
{
  ts->tv_sec = (time_t)seconds;
  ts->tv_nsec = (long)(fmod(seconds, 1.0 ) * BILLION );
}


int WaitForWallClock()
{
  static double min_sleep_time = 0.0;
  static double avg_interval = 0.01;

  struct timespec ts;
  
  static double last_time = 0.0;
  
  double timenow = GetRealTime();
  double interval = timenow - last_time;
  
  last_time += interval;
  
  avg_interval = 0.9 * avg_interval + 0.1 * interval;
  
  double freq = 1.0 / avg_interval;
  
  double outputtime =  GetRealTime();
  //PRINT_DEBUG4( "time %.4f freq %.2f interval %.4f avg %.4f", 
  //	outputtime, freq, interval, avg_interval );
  //printf( "time %.4f freq %.2f interval %.4f avg %.4f\n", 
  //	outputtime, freq, interval, avg_interval );

  //PRINT_MSG2( "time %.4f freq %.2fHz", timenow, freq );
  
  // if we have spare time, go to sleep
  double spare_time = update_interval - avg_interval;
  
  if( spare_time > min_sleep_time )
    {
      //printf( "sleeping for %.6f seconds\n", spare_time );
      ts.tv_sec = (time_t)spare_time;
      ts.tv_nsec = (long)(fmod( spare_time, 1.0 ) * BILLION );
      
      nanosleep( &ts, NULL ); // just stop the powerbook getting hot :)
    }
  //else
  //printf( "not sleeping\n" );
  
  return 0; // success
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
      puts( "" );
      PRINT_ERR( "Server failed to initialize. Quitting." );
      quit = 1;
    }
  
  GuiInit( argc, argv );
  
  puts( "" ); // end the startup output line
  
  // Register callback for quit (^C,^\) events
  // - sig_quit function raises the quit flag 
  signal(SIGINT, sig_quit );
  signal(SIGQUIT, sig_quit );
  signal(SIGTERM, sig_quit );
  signal(SIGHUP, sig_quit );

  // catch clock start/stop commands
  signal(SIGUSR1, CatchSigUsr1 );
  
  //int c = 0;
  
  // create the root object
  //RootEntity(

  // the main loop
  while( !quit ) 
    {
      //printf( "cycle %d\n", c++ );

      // set up new clients
      if( SIOAcceptConnections() == -1 ) break;
      
      // receive commands, property changes and subscriptions from
      // each client. will block until something is read.  if the
      // server receives 'update' commands from all clients, it'll
      // update the world
      if( SIOServiceConnections( &HandleLostConnection,
				 &HandleCommand, 
				 &HandleModel, 
				 &HandleProperty,
				 &HandleGui ) == -1 ) break;
      
      // update the simulation model
      if( CEntity::root && !paused  ) 
	{
	  //PRINT_WARN( "update root" );
	  CEntity::root->Update();
	  CEntity::simtime += update_interval; 
	}
      
      // find all the dirty properties and write them out to subscribers
      if( PublishDirty() == -1 ) break;
      
      // send a continue to all clients so they know we're done talking
      // -1 means ALL CONNECTIONS
      if( SIOWriteMessage( -1, CEntity::simtime, 
			   STG_HDR_CONTINUE, NULL, 0 ) == -1 ) break;
      
      if( GuiUpdate() == -1 ) break;
      
      // if( real_time_mode )
      WaitForWallClock();
      
    }
  // clean up and exit
  StageQuit();
  
  return 0; 
}


