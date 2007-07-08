/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) 2004, 2005 Richard Vaughan
 *                      
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
 * Desc: A plugin driver for Player that gives access to Stage devices.
 * Author: Richard Vaughan
 * Date: 10 December 2004
 * CVS: $Id: p_simulation.cc,v 1.15.2.4 2007-07-08 01:44:09 rtv Exp $
 */

// DOCUMENTATION ------------------------------------------------------------

/** @addtogroup player 
@par Simulation interface
- PLAYER_SIMULATION_REQ_SET_POSE2D
- PLAYER_SIMULATION_REQ_GET_POSE2D
- PLAYER_SIMULATION_REQ_SET_PROPERTY_INT
  - "fiducial_return" 0-32767
  - "laser_return" 0-2
  - "gripper_return" 0-1
  - "ranger_return" 0-1
  - "obstacle_return" 0-1
  - "color" 0xRRGGBB
- PLAYER_SIMULATION_REQ_SET_PROPERTY_FLOAT
  - "mass" 0 <= N
  - "watts" 0 <= N
*/

// CODE ------------------------------------------------------------

#define DEBUG

#include "p_driver.h"

// these are Player globals
extern bool player_quiet_startup;
extern PlayerTime* GlobalTime;

#define DRIVER_ERROR(X) printf( "Stage driver error: %s\n", X )


////////////////////////////////////////////////////////////////////////////////////

// player's cmdline args
//extern int global_argc;
//extern char** global_argv;

// 
// SIMULATION INTERFACE
//
InterfaceSimulation::InterfaceSimulation( player_devaddr_t addr, 
					  StgDriver* driver,
					  ConfigFile* cf, 
					  int section )
  : Interface( addr, driver, cf, section )
{
  printf( "a Stage world" ); fflush(stdout);
  //puts( "InterfaceSimulation constructor" );
  
  // boot libstage, requesting halt on any glib/gtk/gnome problem
//   int argc = 2;
//   char* argv[2];
//   argv[0] = "player";
//   argv[1] = "--g-fatal-warnings";
//   stg_init( argc, argv );

  const char* worldfile_name = cf->ReadString(section, "worldfile", NULL );
  
  if( worldfile_name == NULL )
    {
      PRINT_ERR1( "device \"%s\" uses the Stage driver but has "
		  "no \"model\" value defined. You must specify a "
		  "model name that matches one of the models in "
		  "the worldfile.",
		  worldfile_name );
      return; // error
    }
  
  char fullname[MAXPATHLEN];
  
  if( worldfile_name[0] == '/' )
    strcpy( fullname, worldfile_name );
  else
    {
      char *tmp = strdup(cf->filename);
      snprintf( fullname, MAXPATHLEN, 
		"%s/%s", dirname(tmp), worldfile_name );      
      free(tmp);
    }
  
  // a little sanity testing
  if( !g_file_test( fullname, G_FILE_TEST_EXISTS ) )
    {
      PRINT_ERR1( "worldfile \"%s\" does not exist", worldfile_name );
      return;
    }
  
  // create a passel of Stage models in the local cache based on the
  // worldfile
  
  StgDriver::world = stg_world_create_from_file( 0, fullname );
  assert(StgDriver::world);
  //printf( " done.\n" );
  
  // poke the P/S name into the window title bar
  if( StgDriver::world && StgDriver::world->win )
    {
      char txt[128];
      snprintf( txt, 128, "Player/Stage: %s", StgDriver::world->token );
      stg_world_set_title(StgDriver::world, txt ); 
    }

  // steal the global clock - a bit aggressive, but a simple approach
  if( GlobalTime ) delete GlobalTime;
  assert( (GlobalTime = new StgTime( driver ) ));
  
  // start the simulation
  // printf( "  Starting world clock... " ); fflush(stdout);
  //stg_world_resume( world );
  
  StgDriver::world->paused = FALSE;
  
  // this causes Driver::Update() to be called even when the device is
  // not subscribed
  driver->alwayson = TRUE;    
  
  // Start the device thread; spawns a new thread and executes
  // StgDriver::Main(), which contains the main loop for the driver.
  //puts( "\nStarting thread" );
  //driver->StartThread();

  puts( "" ); // end the Stage startup line
}      

int InterfaceSimulation::ProcessMessage(MessageQueue* resp_queue,
                                        player_msghdr_t* hdr,
                                        void* data)
{
  // Is it a request to set a model's pose?
  if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, 
                           PLAYER_SIMULATION_REQ_SET_POSE2D, 
                           this->addr))
  {
    player_simulation_pose2d_req_t* req = 
            (player_simulation_pose2d_req_t*)data;

    stg_pose_t pose;
    pose.x = req->pose.px;
    pose.y = req->pose.py;
    pose.a = req->pose.pa;

    // look up the named model

    StgModel* mod = 
            stg_world_model_name_lookup( StgDriver::world, req->name );

    if( mod )
      {
	printf( "Stage: moving \"%s\" to (%.2f,%.2f,%.2f)\n",
		req->name, pose.x, pose.y, pose.a );
	
	mod->SetPose( &pose );
	
	this->driver->Publish(this->addr, resp_queue,
			      PLAYER_MSGTYPE_RESP_ACK,
			      PLAYER_SIMULATION_REQ_SET_POSE2D);
	return(0);
      }
    else
      {
	PRINT_WARN1( "SETPOSE2D request: simulation model \"%s\" not found", req->name );
	return(-1);
      }
  }
  // Is it a request to set a model's pose?
  else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, 
				PLAYER_SIMULATION_REQ_SET_PROPERTY, 
				this->addr))
    {

      player_simulation_property_req_t* req = 
	(player_simulation_property_req_t*)data;
      
      // look up the named model      
      StgModel* mod = 
	stg_world_model_name_lookup( StgDriver::world, req->name );
      
      if( mod )
	{
	  int ack = 
	    mod->SetProperty( req->prop, 
			      (void*)req->value );
	  
	  this->driver->Publish(this->addr, resp_queue,
				ack==0 ? PLAYER_MSGTYPE_RESP_ACK : PLAYER_MSGTYPE_RESP_NACK,
				PLAYER_SIMULATION_REQ_SET_PROPERTY);
	  return(0);
	}
      else
	{
	  PRINT_WARN1( "SET_PROPERTY request: simulation model \"%s\" not found", req->name );
	  return(-1);
	}
    }
  // Is it a request to get a model's pose?
  else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, 
                                PLAYER_SIMULATION_REQ_GET_POSE2D, 
                                this->addr))
    {
      player_simulation_pose2d_req_t* req = 
	(player_simulation_pose2d_req_t*)data;
      
      printf( "Stage: received request for position of object \"%s\"\n", req->name );
      
      // look up the named model	
      StgModel* mod = 
	stg_world_model_name_lookup( StgDriver::world, req->name );
      
      if( mod )
	{
	  stg_pose_t pose;
	  mod->GetPose( &pose );

      printf( "Stage: returning location (%.2f,%.2f,%.2f)\n",
              pose.x, pose.y, pose.a );


      player_simulation_pose2d_req_t reply;
      memcpy( &reply, req, sizeof(reply));
      reply.pose.px = pose.x;
      reply.pose.py = pose.y;
      reply.pose.pa = pose.a;

      /*
         printf( "Stage: returning location (%d %d %d)\n",
         reply.x, reply.y, reply.a );
       */

      this->driver->Publish( this->addr, resp_queue, 
                             PLAYER_MSGTYPE_RESP_ACK, 
                             PLAYER_SIMULATION_REQ_GET_POSE2D,
                             (void*)&reply, sizeof(reply), NULL );
      return(0);
    }
    else
    {
      PRINT_WARN1( "Stage: GET_POSE2D request: simulation model \"%s\" not found", req->name );
      return(-1);
    }
  }
  else
  {
    // Don't know how to handle this message.
    PRINT_WARN2( "stg_simulation doesn't support msg with type/subtype %d/%d",
		 hdr->type, hdr->subtype);
    return(-1);
  }
}
