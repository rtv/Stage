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
 * CVS: $Id: p_simulation.cc,v 1.3 2005-07-27 21:13:28 rtv Exp $
 */

// DOCUMENTATION ------------------------------------------------------------

// TODO - configs I should implement
//  - PLAYER_SONAR_POWER_REQ
//  - PLAYER_BLOBFINDER_SET_COLOR_REQ
//  - PLAYER_BLOBFINDER_SET_IMAGER_PARAMS_REQ

// CODE ------------------------------------------------------------

#define DEBUG

#include "p_driver.h"

// these are Player globals
extern bool quiet_startup;
extern PlayerTime* GlobalTime;

#define DRIVER_ERROR(X) printf( "Stage driver error: %s\n", X )

////////////////////////////////////////////////////////////////////////////////////

Interface::Interface(  player_device_id_t id, 
		       StgDriver* driver,
		       ConfigFile* cf, 
		       int section )
{
  //puts( "Interface constructor" );

  this->id = id;
  this->driver = driver;
  //this->cf = cf;
  //this->section = section;

  this->cmd_len = 0;
  this->data_len = 0;
  this->req_qlen = 10;
  this->rep_qlen = 10;
}

////////////////////////////////////////////////////////////////////////////////////


// 
// SIMULATION INTERFACE
//
InterfaceSimulation::InterfaceSimulation( player_device_id_t id, 
					  StgDriver* driver,
					  ConfigFile* cf, 
					  int section )
  : Interface( id, driver, cf, section )
{
  printf( "Simulated world" ); fflush(stdout);
  //puts( "InterfaceSimulation constructor" );

  // boot libstage 
  //stg_init( global_argc, global_argv );
  
  
  this->data_len = sizeof(player_simulation_data_t);
  this->cmd_len = sizeof(player_simulation_cmd_t);

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
  
  //if( !quiet_startup )
    {
      printf( " [%s]", fullname );      
      fflush(stdout);
    }

  StgDriver::world = stg_world_create_from_file( fullname );
  assert(StgDriver::world);
  //printf( " done.\n" );
  
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

void SimulationData( Interface* device, void* data, size_t len )
{
  PRINT_WARN( "refresh data requested for simulation device - devices produces no data" );
  device->driver->PutData( device->id, NULL, 0, NULL); 
}

void InterfaceSimulation::Configure( void* client, void* buffer, size_t len )
{
  //printf("got simulation request\n");
  
  if( ! buffer )
    {
      PRINT_WARN( "configure request has NULL data" );
      return;
    }

  if( len == 0 )
    {
      PRINT_WARN( "configure request has zero length" );
      return;
    }
    
  
  // switch on the config type (first byte)
  uint8_t* buf = (uint8_t*)buffer;
  switch( buf[0] )
    {        
      // if Player has defined this config, implement it
#ifdef PLAYER_SIMULATION_SET_POSE2D
    case PLAYER_SIMULATION_SET_POSE2D:
      if( len != sizeof(player_simulation_pose2d_req_t) )
	PRINT_WARN2( "POSE2D request is wrong size (%d/%d bytes)\n",
		     len,sizeof(player_simulation_pose2d_req_t)); 
      else
	{
	  player_simulation_pose2d_req_t* req = (player_simulation_pose2d_req_t*)buffer;
	  
	  stg_pose_t pose;
	  pose.x = (int32_t)ntohl(req->x) / 1000.0;
	  pose.y = (int32_t)ntohl(req->y) / 1000.0;
	  pose.a = DTOR( ntohl(req->a) );
	  
	  printf( "Stage: received request to move object \"%s\" to (%.2f,%.2f,%.2f)\n",
		  req->name, pose.x, pose.y, pose.a );
	  
	  // look up the named model
	  
	  stg_model_t* mod = 
	    stg_world_model_name_lookup( StgDriver::world, req->name );
 	
	  if( mod )
	    {
	      // move it 
	      printf( "moving model \"%s\"\n", req->name );	    
	      stg_model_set_property( mod, "pose", &pose, sizeof(pose));
	      this->driver->PutReply( this->id, client, PLAYER_MSGTYPE_RESP_ACK, NULL );
	    }
	  else
	    {
	      PRINT_WARN1( "SETPOSE2D request: simulation model \"%s\" not found", req->name );
	      this->driver->PutReply( this->id, client, PLAYER_MSGTYPE_RESP_NACK, NULL );
	    }
	}         
      break;
#endif
          // if Player has defined this config, implement it
#ifdef PLAYER_SIMULATION_GET_POSE2D
    case PLAYER_SIMULATION_GET_POSE2D:
      if( len != sizeof(player_simulation_pose2d_req_t) )
	PRINT_WARN2( "GET_POSE2D request is wrong size (%d/%d bytes)\n",
		     len,sizeof(player_simulation_pose2d_req_t)); 
      else
	{
	  player_simulation_pose2d_req_t* req = (player_simulation_pose2d_req_t*)buffer;

	  printf( "Stage: received request for position of object \"%s\"\n", req->name );
	  
	  // look up the named model	
	  stg_model_t* mod = 
	    stg_world_model_name_lookup( StgDriver::world, req->name );
	  
	  if( mod )
	    {
	      stg_pose_t* pose = (stg_pose_t*)
		stg_model_get_property_fixed( mod, "pose", sizeof(stg_pose_t));
	      assert(pose);
	      
	      
	      printf( "Stage: returning location (%.2f,%.2f,%.2f)\n",
		      pose->x, pose->y, pose->a );
	      
	      
	      player_simulation_pose2d_req_t reply;
	      memcpy( &reply, req, sizeof(reply));
	      reply.x = htonl((int32_t)(pose->x*1000.0));
	      reply.y = htonl((int32_t)(pose->y*1000.0));
	      reply.a = htonl((int32_t)RTOD(pose->a));
	      
	      /*
		printf( "Stage: returning location (%d %d %d)\n",
		reply.x, reply.y, reply.a );
	      */
	      
	      this->driver->PutReply( this->id, client, PLAYER_MSGTYPE_RESP_ACK, 
				      &reply, sizeof(reply),  NULL );
	    }
	  else
	    {
	      PRINT_WARN1( "Stage: GETPOSE2D request: simulation model \"%s\" not found", req->name );
	      this->driver->PutReply( this->id, client, PLAYER_MSGTYPE_RESP_NACK, NULL );
	    }
	}      
      break;
#endif
      
    default:      
      PRINT_WARN1( "Stage: simulation device doesn't implement config code %d.", buf[0] );
      if (this->driver->PutReply( this->id, client, PLAYER_MSGTYPE_RESP_NACK, NULL, 0, NULL) != 0)
	DRIVER_ERROR("PutReply() failed");  
      break;
    }
}

