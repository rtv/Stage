/******************************************************************************
* File: irdevice.cc
* Author: Richard Vaughan
* Date: 22 October 2001
* Desc: Presents a single interface to multiple IDAR Devices
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*
* CVS info:
* $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/idarturretdevice.cc,v $
* $Author: rtv $
* $Revision: 1.11 $
******************************************************************************/

#include <math.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "world.hh"
#include "idarturretdevice.hh"


#define DEBUG
//#undef DEBUG

// register this device type with the Library
CEntity idarturret_bootstrap( "idarturret", 
			      IDARTurretType, 
			      (void*)&CIdarTurretDevice::Creator ); 

// constructor 
CIdarTurretDevice::CIdarTurretDevice(CWorld *world, CEntity *parent )
  : CPlayerEntity(world, parent )
{
  stage_type = IDARTurretType;
  m_player.code = PLAYER_IDARTURRET_CODE; // from player's messages.h

  // we're invisible 
  laser_return = LaserTransparent;
  sonar_return = false;
  obstacle_return = false;
  idar_return = IDARTransparent;

  // set the Player IO sizes correctly for this type of Entity
  m_data_len    = 0;
  m_command_len = 0;
  m_config_len  = 1;
  m_reply_len  = 1;
  
    
  this->color = ::LookupColor(IDAR_COLOR);

  size_x = 0.08; // this is the actual physical size of the HRL device
  size_y = 0.08; // but can be changed in the world file to suit

  // Set the default shape
  shape = ShapeCircle;

  m_interval = 0.001; // updates frequently,
  // but only has work to do when it receives a command
  // so it's not a CPU hog unless we send it lots of messages
 
  
  double angle_per_idar = M_PI * 2 / PLAYER_IDARTURRET_IDAR_COUNT;
  double radius = size_x / 2.0;

  // create our child devices
  // we don't call CEntity::Load() for our children - we'll configure them here
  for( int i=0; i<PLAYER_IDARTURRET_IDAR_COUNT; i++ )
    {
      idars[i] = new CIdarDevice( world, this );       // create

      // set the player
      idars[i]->m_player.index = i;

      double angle = angle_per_idar * i;
      idars[i]->SetPose( radius * cos(angle), radius * sin(angle), angle ); // place
    } 

  //m_num_scanlines = IDAR_SCANLINES; 
  //m_angle_per_sensor = IDAR_FOV;
  //m_angle_per_scanline = m_angle_per_sensor / m_num_scanlines;
}



void CIdarTurretDevice::Sync( void ) 
{
  CPlayerEntity::Sync();

  void *client;
  player_idarturret_config_t cfg;
  player_idarturret_reply_t reply;
  
  // Get config
  int res = GetConfig( &client, &cfg, sizeof(cfg));
  
  switch( res )
    { 
    case 0:
      // nothing available - nothing to do
      break;
      
    case -1: // error
      PRINT_ERR( "get config failed" );
      break;
      
    case sizeof(cfg): // the size we expect - we received a config request
      // what does the client want us to do?
      switch( cfg.instruction )
	{
	case IDAR_TRANSMIT:
	  PutReply(client, PLAYER_MSGTYPE_RESP_ACK );
	  
	  //puts( "TX" );
	  for( int i=0; i<PLAYER_IDARTURRET_IDAR_COUNT; i++ )
	    // if there is a message to send in this direction, send it
	    if( cfg.tx[i].len > 0 )
	      idars[i]->TransmitMessage( &(cfg.tx[i]) );
	  
	  break;
	  
	case IDAR_RECEIVE:
	  //puts( "RX" );
	  // gather and wipe the current messages from each idar
	  for( int i=0; i<PLAYER_IDARTURRET_IDAR_COUNT; i++ )
	    idars[i]->CopyAndClearMessage( &reply.rx[i] );
	  
	  PutReply(client, PLAYER_MSGTYPE_RESP_ACK,NULL,&reply,sizeof(reply));
	  break;
	  
	case IDAR_RECEIVE_NOFLUSH:
	  //puts( "RX_NF" );
	  // gather current message from each idar
	  for( int i=0; i<PLAYER_IDARTURRET_IDAR_COUNT; i++ )
	    idars[i]->CopyMessage( &reply.rx[i] );
	  
	  PutReply(client, PLAYER_MSGTYPE_RESP_ACK,NULL,&reply,sizeof(reply));
	  // send back the currently stored message
	  
	  break;

	  // both at once for efficiency
	case IDAR_TRANSMIT_RECEIVE:
	  //puts( "TX" );
	  for( int i=0; i<PLAYER_IDARTURRET_IDAR_COUNT; i++ )
	    // if there is a message to send in this direction, send it
	    if( cfg.tx[i].len > 0 )
	      idars[i]->TransmitMessage( &(cfg.tx[i]) );
	  
	  // puts( "RX" );
	  for( int i=0; i<PLAYER_IDARTURRET_IDAR_COUNT; i++ )
	    idars[i]->CopyAndClearMessage( &reply.rx[i] );

	  PutReply(client, PLAYER_MSGTYPE_RESP_ACK,NULL,&reply,sizeof(reply));
	  break;
	  
	  
	default:
	  printf( "STAGE warning: unknown idarturret config instruction %d\n",
		  cfg.instruction );
	}
      break;
      
    default: // a bad value
      PRINT_ERR1( "wierd: idarturret config returned %d ", res );
      break;
    }
}

void CIdarTurretDevice::Update( double sim_time ) 
{
  //  puts( "UPDATE" );

  CPlayerEntity::Update( sim_time ); // inherit some debug output
  
  // UPDATE OUR RENDERING
  double x, y, th;
  GetGlobalPose( x,y,th );
  
  ReMap( x, y, th );
  
  // render our children in the world
  for( int i=0; i<PLAYER_IDARTURRET_IDAR_COUNT; i++ )
    idars[i]->Update( sim_time );

  // dump out if noone is subscribed
  if(!Subscribed())
      return; 

  m_last_update = sim_time; 
}

#ifdef INCLUDE_RTK2

///////////////////////////////////////////////////////////////////////////
// Initialise the rtk gui
void CIdarTurretDevice::RtkStartup()
{
  CPlayerEntity::RtkStartup();

  //this->data_fig = rtk_fig_create(m_world->canvas, this->fig, 49);
 
  //rtk_fig_origin(this->data_fig, 0,0,0 );
   
  // Set the color
  //rtk_fig_color_rgb32(this->data_fig, RGB(200,200,200) );
 
  for( int i=0; i<PLAYER_IDARTURRET_IDAR_COUNT; i++ )
  {
    idars[i]->RtkStartup();
    // don't display the idar body in the turret (saves some time)
    rtk_fig_clear( idars[i]->fig );
  }
  
}


///////////////////////////////////////////////////////////////////////////
// Finalise the rtk gui
void CIdarTurretDevice::RtkShutdown()
{
  for( int i=0; i<PLAYER_IDARTURRET_IDAR_COUNT; i++ )
  idars[i]->RtkShutdown();

  //if(this->data_fig) rtk_fig_destroy(this->data_fig);
  
  CPlayerEntity::RtkShutdown();
} 


///////////////////////////////////////////////////////////////////////////
// Update the rtk gui
void CIdarTurretDevice::RtkUpdate()
{
  CPlayerEntity::RtkUpdate();
   
  // for( int i=0; i<PLAYER_IDARTURRET_IDAR_COUNT; i++ )
  //idars[i]->RtkUpdate();
}

#endif
