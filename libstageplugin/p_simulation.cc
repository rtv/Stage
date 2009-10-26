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
 * CVS: $Id: p_simulation.cc,v 1.4 2008-02-21 19:52:24 rtv Exp $
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

//#define DEBUG

#include <libgen.h> // for dirname(3)
#include <libplayercore/globals.h> // for player_argc & player_argv

#include "p_driver.h"
using namespace Stg;

// these are Player globals
extern bool player_quiet_startup;
extern PlayerTime* GlobalTime;

#define DRIVER_ERROR(X) printf( "Stage driver error: %s\n", X )


////////////////////////////////////////////////////////////////////////////////////

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

  Stg::Init( &player_argc, &player_argv );

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
	// XX TODO
	//  if( !g_file_test( fullname, G_FILE_TEST_EXISTS ) )
	//{
	//  PRINT_ERR1( "worldfile \"%s\" does not exist", worldfile_name );
	//  return;
	//}

  // create a passel of Stage models in the local cache based on the
  // worldfile

  // if the initial size is to large this crashes on some systems
  StgDriver::world = new WorldGui( 400, 300, "Player/Stage" );
  assert(StgDriver::world);

  puts("");
  StgDriver::world->Load( fullname );
  //printf( " done.\n" );

  // poke the P/S name into the window title bar
//   if( StgDriver::world )
//     {
//       char txt[128];
//       snprintf( txt, 128, "Player/Stage: %s", StgDriver::world->token );
//       StgDriverstg_world_set_title(StgDriver::world, txt );
//     }

  // steal the global clock - a bit aggressive, but a simple approach

  delete GlobalTime;
  GlobalTime = new StTime( driver );
  assert(GlobalTime);
  // start the simulation
  // printf( "  Starting world clock... " ); fflush(stdout);
  //stg_world_resume( world );

  StgDriver::world->Start();

  // this causes Driver::Update() to be called even when the device is
  // not subscribed
  driver->alwayson = TRUE;

  puts( "" ); // end the Stage startup line
}

int InterfaceSimulation::ProcessMessage(QueuePointer &resp_queue,
                                        player_msghdr_t* hdr,
                                        void* data)
{
	if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, PLAYER_CAPABILTIES_REQ, addr))
	{
		PLAYER_ERROR1("%p\n", data);
	    player_capabilities_req_t & cap_req = * reinterpret_cast<player_capabilities_req_t *> (data);
	    if (cap_req.type == PLAYER_MSGTYPE_REQ && (cap_req.subtype == PLAYER_SIMULATION_REQ_SET_POSE3D || cap_req.subtype == PLAYER_SIMULATION_REQ_GET_POSE3D))
	    {
	    	this->driver->Publish(addr, resp_queue, PLAYER_MSGTYPE_RESP_ACK, PLAYER_CAPABILTIES_REQ);
	    	return 0;
	    }
	}

	// Is it a request to get a model's pose in 2D?
	if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ,
								  PLAYER_SIMULATION_REQ_GET_POSE2D,
								  this->addr))
    {
		player_simulation_pose2d_req_t* req =
			(player_simulation_pose2d_req_t*)data;

		PRINT_DEBUG1( "Stage: received request for the 2D position of object \"%s\"\n", req->name );

		// look up the named model
		Model* mod = StgDriver::world->GetModel( req->name );

		if( mod )
		{
			Pose pose = mod->GetPose();

			PRINT_DEBUG3( "Stage: returning location [ %.2f, %.2f, %.2f ]\n",
						  pose.x, pose.y, pose.a );

			player_simulation_pose2d_req_t reply;
			memcpy( &reply, req, sizeof(reply));
			reply.pose.px = pose.x;
			reply.pose.py = pose.y;
			reply.pose.pa = pose.a;

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

	// Is it a request to set a model's pose in 2D?
	if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ,
						   PLAYER_SIMULATION_REQ_SET_POSE2D,
						   this->addr))
	{
		player_simulation_pose2d_req_t* req =
				(player_simulation_pose2d_req_t*)data;

		// look up the named model
		Model* mod = StgDriver::world->GetModel( req->name );

		if( mod )
		{
			PRINT_DEBUG4( "Stage: moving \"%s\" to [ %.2f, %.2f, %.2f ]\n",
						  req->name, req->pose.px, req->pose.py, req->pose.pa );

			Pose pose = mod->GetPose();
			pose.x = req->pose.px;
			pose.y = req->pose.py;
			pose.a = req->pose.pa;

			mod->SetPose( pose );

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

	// Is it a request to get a model's pose in 3D?
	else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ,
								  PLAYER_SIMULATION_REQ_GET_POSE3D,
								  this->addr))
    {
		player_simulation_pose3d_req_t* req =
		(player_simulation_pose3d_req_t*)data;

		PRINT_DEBUG1( "Stage: received request for the 3D position of object \"%s\"\n", req->name );

		// look up the named model
		Model* mod = StgDriver::world->GetModel( req->name );

		if( mod )
		{
			Pose pose = mod->GetPose();

			PRINT_DEBUG4( "Stage: returning location [ %.2f, %.2f, %.2f, %.2f ]\n",
						  pose.x, pose.y, pose.z, pose.a );

			player_simulation_pose3d_req_t reply;
			memcpy( &reply, req, sizeof(reply));
			reply.pose.px = pose.x;
			reply.pose.py = pose.y;
			reply.pose.pz = pose.z;
			reply.pose.proll = 0; // currently unused
			reply.pose.ppitch = 0; // currently unused
			reply.pose.pyaw = pose.a;
			reply.simtime = mod->GetWorld()->SimTimeNow(); // time in microseconds

			this->driver->Publish( this->addr, resp_queue,
								  PLAYER_MSGTYPE_RESP_ACK,
								  PLAYER_SIMULATION_REQ_GET_POSE3D,
								  (void*)&reply, sizeof(reply), NULL );
			return(0);
		}
		else
		{
			PRINT_WARN1( "Stage: GET_POSE3D request: simulation model \"%s\" not found", req->name );
			return(-1);
		}
	}

	// Is it a request to set a model's pose in 3D?
	if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ,
							 PLAYER_SIMULATION_REQ_SET_POSE3D,
							 this->addr))
	{
		player_simulation_pose3d_req_t* req =
		(player_simulation_pose3d_req_t*)data;

		// look up the named model
		Model* mod = StgDriver::world->GetModel( req->name );

		if( mod )
		{
			PRINT_DEBUG5( "Stage: moving \"%s\" to [ %.2f, %.2f, %.2f %.2f ]\n",
						  req->name, req->pose.px, req->pose.py, req->pose.pz, req->pose.pyaw );

			Pose pose = mod->GetPose();
			pose.x = req->pose.px;
			pose.y = req->pose.py;
			pose.z = req->pose.pz;
			pose.a = req->pose.pyaw;
			// roll and pitch are unused

			mod->SetPose( pose );

			this->driver->Publish(this->addr, resp_queue,
								  PLAYER_MSGTYPE_RESP_ACK,
								  PLAYER_SIMULATION_REQ_SET_POSE3D);
			return(0);
		}
		else
		{
			PRINT_WARN1( "SETPOSE2D request: simulation model \"%s\" not found", req->name );
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
