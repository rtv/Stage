/******************************************************************************
* File: irdevice.cc
* Author: Richard Vaughan
* Date: 22 October 2001
* Desc: Simulates a single sensor of HRL's Infrared Data and Ranging Device
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
* $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/idardevice.cc,v $
* $Author: gerkey $
* $Revision: 1.6 $
******************************************************************************/


#include <math.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <iostream>

#include "world.hh"
#include "idardevice.hh"
#include "raytrace.hh"

#define DEBUG
//#undef DEBUG

// control display of player index in device body
//#define RENDER_INDEX
//#define RENDER_SCANLINES
//#define SHOW_MSGS

#define IDAR_TRANSMIT_ANGLE        M_PI/4.0
#define IDAR_RECEIVE_ANGLE        M_PI/4.0
#define IDAR_SCANLINES        5
#define IDAR_MAX_RANGE       1.0

// handy circular normalization macros
#define NORMALIZECIRCULAR_INT(value,max) (((value%max)+max)%max)
#define NORMALIZECIRCULAR_DOUBLE(value,max) fmod( fmod(value,max)+max, max)

#define NORMALIZERADIANS(value)  NORMALIZECIRCULAR_DOUBLE(value,TWOPI)
#define NORMALIZEDEGREES(value)  NORMALIZECIRCULAR_DOUBLE(value,360.0)


// register this device type with the Library
CEntity idar_bootstrap( "idar", 
			IDARType, 
			(void*)&CIdarDevice::Creator ); 

// constructor 
CIdarDevice::CIdarDevice(CWorld *world, CEntity *parent )
  : CPlayerEntity(world, parent )
{
  stage_type = IDARType;

  // we're invisible except to other IDARs
  laser_return = LaserTransparent;
  sonar_return = false;
  obstacle_return = false;
  idar_return = IDARReceive;

  // set the Player IO sizes correctly for this type of Entity
  m_data_len    = 0;//sizeof( player_idar_data_t );
  m_command_len = 0;//sizeof( player_idar_command_t );
  m_config_len  = 1;
  m_reply_len  = 1;
  

  m_player.code = PLAYER_IDAR_CODE; // from player's messages.h
    
  this->color = ::LookupColor(IDAR_COLOR);

  m_max_range = IDAR_MAX_RANGE;
    
  size_x = 0.03; // this is the actual physical size of the HRL device
  size_y = 0.02; // but can be changed in the world file to suit

  // Set the default shape
  shape = ShapeRect;

  m_interval = 0.001; // updates frequently,
  // but only has work to do when it receives a command
  // so it's not a CPU hog unless we send it lots of messages
 
  // init the message buffer
  memset( &recv, 0, sizeof(recv) );

  m_num_scanlines = IDAR_SCANLINES; 
  m_angle_per_sensor = IDAR_TRANSMIT_ANGLE;
  m_angle_per_scanline = m_angle_per_sensor / m_num_scanlines;
}

///////////////////////////////////////////////////////////////////////////
// Startup routine
//
bool CIdarDevice::Startup()
{
  if (!CPlayerEntity::Startup())
    return false;

  SetDriverName("hrl_idar");
  return true;
}

void CIdarDevice::Sync( void )
{
  // sync our children
  CPlayerEntity::Sync();

  void *client;
  player_idar_config_t cfg;
  
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
	  //puts( "TX" );
	  TransmitMessage( &(cfg.tx) );
	  PutReply(client, PLAYER_MSGTYPE_RESP_ACK );
	  
	  break;
	  
	case IDAR_RECEIVE:
	  //puts( "RX" );
	  // send back the currently stored message
	  PutReply(client, PLAYER_MSGTYPE_RESP_ACK, NULL, &recv, sizeof(recv));
	  ClearMessage(); // wipe the buffer
	  break;
	  
	case IDAR_RECEIVE_NOFLUSH:
	  //puts( "RX_NF" );
	  // send back the currently stored message
	  PutReply(client, PLAYER_MSGTYPE_RESP_ACK, NULL, &recv, sizeof(recv));
	  // don't clear the current mesg
	  break;
	  
	  // two in one for efficiency!
	case IDAR_TRANSMIT_RECEIVE:
	  TransmitMessage( &(cfg.tx) );
	  PutReply(client, PLAYER_MSGTYPE_RESP_ACK, NULL, &recv, sizeof(recv));
	  ClearMessage(); // wipe the buffer
	  break;

	default:
	  printf( "STAGE warning: unknown idar config instruction %d\n",
		  cfg.instruction );
	}
      break;
      
    default: // a bad value
      PRINT_ERR1( "wierd: idar config returned %d ", res );
      break;
    }
}


// NOTE unlike most devices, live data is not published here - that's
// done in ReceiveMessage()
void CIdarDevice::Update( double sim_time ) 
{
  CPlayerEntity::Update( sim_time ); // inherit some debug output
  
  // UPDATE OUR RENDERING
  double x, y, th;
  GetGlobalPose( x,y,th );
  
  ReMap( x, y, th );
  
  // dump out if noone is subscribed
  if(!Subscribed())
    {
      return; 
    }
  
  m_last_update = sim_time;
}

void CIdarDevice::ClearMessage( void )
{
  // wipe the message (zero the buffer)
  memset( &recv, 0, sizeof(recv) );
#ifdef INCLUDE_RTK2
  // unrender the message 
  rtk_fig_clear(this->data_fig);
#endif
}

void CIdarDevice::TransmitMessage( idartx_t* transmit )
{
  // really should be a valid message here
  assert( transmit && (transmit->len > 0) );
  
  //#ifdef DEBUG
  //printf( "\nTRANSMITTING %d bytes at %p - ",
  //transmit->len, transmit->mesg );
  
  //for( int f=0; f<transmit->len; f++ )
  //printf( "%d ", transmit->mesg[f] );
  //puts ( "" );
  //#endif
  
  // now perform the transmission
  double ox, oy, oth;
  GetGlobalPose( ox, oy, oth ); // start position for the ray trace

#ifdef INCLUDE_RTK2
  rtk_fig_clear(this->rays_fig);
#endif
  
  double scan_start_angle = 
    (oth - m_angle_per_sensor/2.0) + m_angle_per_scanline/2.0;
    
    for( int scanline=0; scanline < m_num_scanlines; scanline++ )
      {
	
	// Compute parameters of scan line
	double scanline_bearing = 
	scan_start_angle + scanline * m_angle_per_scanline;
	
      // normalize to make things easier to read
      scanline_bearing = fmod( scanline_bearing, TWOPI );
	      
      CLineIterator lit( ox, oy, scanline_bearing, m_max_range,
			 m_world->ppm, m_world->matrix,
			 PointToBearingRange );
	      
      //printf( "direction %d scanline %d "
      //      "from %.2f,%.2f %.2fm at %.2f radians\n", 
      //      s, scanline, ox, oy, m_max_range, scanline_bearing );
	      
      CEntity* ent;

      double range = m_max_range;
	      
      // get the next thing in our beam, until the beam is blocked
      while( (ent = lit.GetNextEntity() ) ) 
	{
	  //printf( "HIT %p type %d idar_return %d (i am %p,%d,%d)\n",
	  //ent, ent->m_stage_type, ent->idar_return,
	  //(CEntity*)this, m_stage_type, idar_return );
		
	  //printf( "IDARReceive == %d\n", IDARReceive );

	  // if it's not me or my parent or a sibling and it's not
	  // transparent to idar, i'll try sending a message to it
	  if( (ent != (CEntity*)this) && 
	      (ent != this->m_parent_entity) &&
	      (ent->m_parent_entity != this->m_parent_entity) &&
	      ( ent->idar_return != IDARTransparent) )
	    {		    
	      range = lit.GetRange();
	      assert( range >= 0 );
		   
	      //printf( "s:%d.%d range: %.2f\n",
	      //    s, scanline, range );

	      //printf( "hit entity %p (%d:%d:%d)\n",
	      //     ent, ent->m_player.port,  ent->m_player.code,
	      //     ent->m_player.index );

	      uint8_t intensity;
		    
	      switch( ent->idar_return )
		{
		case IDARReceive: // it's a receiver
		  // attempt to poke this message into his receiver
		  //printf( "TRANSMIT to %p type %d idar_return %d\n",
				//ent, ent->m_stage_type, ent->idar_return );
			
			
		  //PRINT_DEBUG1( "POKING A MESSAGE INTO %p", ent );

		  if( (intensity = LookupIntensity(0,range,false)) > 0 )
		    ((CIdarDevice*)ent)->
		      ReceiveMessage( (CEntity*)this, 
				      transmit->mesg,
				      transmit->len,
				      intensity,
				      false );
		  
		  // continue to next case, since IR receivers are
		  // also reflectors.
		  //break;
		  
		case IDARReflect: // it's a reflector (ie. an obstacle)
		  //printf( "REFLECT from %p type %d idar_return %d\n",
		  //	ent, ent->stage_type, ent->idar_return );
			
		  // try poking this message into my own receiver
		  if( (intensity = LookupIntensity(0,range,true)) > 0 )
		    ReceiveMessage( (CEntity*)this, 
				    transmit->mesg,
				    transmit->len,
				    intensity, 
				    true );
		  break;
			
		default:
		  printf( "STAGE WARNING: UNKNOWN IDAR VALUE %d\n", 
			  ent->idar_return );
		}
		  
	      break; // out of the while loop because we hit something
	    }
	}
#ifdef INCLUDE_RTK2
#ifdef RENDER_SCANLINES
      rtk_fig_color_rgb32(this->rays_fig, RGB(220,220,220) );	
      rtk_fig_arrow(this->rays_fig, 0,0, scanline_bearing-oth, range, 0.03);
#endif
#endif    
    }
}


bool CIdarDevice::ReceiveMessage( CEntity* sender,
				  unsigned char* mesg, int len, 
				  uint8_t intensity,
				  bool reflection )
{
  //PRINT_DEBUG( "RECEIVE MESSAGE" );

  //printf( "mesg recv - sensor: %d  len: %d  intensity: %d  refl: %d\n",
  //  sensor, len, intensity, reflection );

  // TEST INTENSITY
  // we only accept this message if it's the most intense thing we've seen
  if( intensity < recv.intensity )
    return false;

  
  // TEST REFLECTION AND  ANGLE OF INCIDENCE
  // we accept reflections without checking angle
  if( !reflection ) 
    { // it's not a reflection, so we check the angle of incidence 
      double sx, sy, sa; // the sender's pose
      double lx, ly, la; // my pose
      
      sender->GetGlobalPose( sx, sy, sa );
      this->GetGlobalPose( lx, ly, la );
      
      double incidence = atan2( sy-ly, sx-lx ) - la;
      NORMALIZE(incidence);
      
      //printf( "%p G: %.2f,%.2f,%.2f   L: %.2f,%.2f,%.2f inc: %.2f\n", 
      //      this, sx, sy, sa, lx, ly, la, incidence );
      
      // if it's out of our receptive angle, we reject it and bail.
      if( fabs(incidence) > IDAR_RECEIVE_ANGLE/2.0 )
	return false;
    }
  
  
  // looks good  - accept the message
  // copy the message into the data buffer
  memcpy( &recv.mesg, mesg, len );
  
  recv.len = len;
  recv.intensity = intensity;
  recv.reflection = (uint8_t)reflection;
  
  // record the time we received this message
  recv.timestamp_sec = m_world->m_sim_timeval.tv_sec;
  recv.timestamp_usec = m_world->m_sim_timeval.tv_usec;
  
#ifdef INCLUDE_RTK2
      
  // reflections are green, remote messages are red
  rtk_fig_color_rgb32(this->data_fig, 
		      reflection ? RGB(0,200,0) : RGB(200,0,0) );
  
  rtk_fig_arrow(this->data_fig, 0,0,0, 1.5*size_x, size_y );
  
#ifdef SHOW_MSGS
  
  // room for the message in hex text
  char message[ 3 * IDARBUFLEN + 6];
  
  // print the message in hex, with a space between each char
  for( int c=0; c<recv.len; c++ )
    sprintf( message + 3*c, "%2X ", recv.mesg[c] );
  message[ 3 * recv.len ] = 0; // terminate
  
  // add the intensity
  sprintf( message, "%s (%d)", message, recv.intensity );
  
  
  rtk_fig_text(this->data_fig, 0,0,0, message);
#endif
#endif

  return true; // we accepted the message
}



// HERE ARE A BUNCH OF RANGE/INTENSITY CONVERSION TABLES DETERMINED
// EMPIRICALLY BY EXPERIMENTS WITH THE PHERBOTS. WE USE ONE OF THESE
// FOR ROBOT-TO-ROBOT TRANSMISSIONS AND ONE FOR REFLECTIONS

// ranges are in INCHES

// (BASED ON 3g32)
int reflection_intensity[] = { 150,129,117,115,112,104,103,102,101,100};
int reflection_range[]     = {   0,  1,  7, 10, 14, 26, 28, 31, 35, 39, 99};
int reflection_len = 10;

// (BASED ON HERE_I_AM)
int direct_intensity[] = { 130,126,125,123,122,121,120,119,118,117,115,114,
			   112,112,111,109,108,107,106,105,104,103,101, 97,
			   96, 95, 94, 93, 90 };
int direct_range[]     = {   6,  9, 12, 15, 18, 21, 24, 28, 32, 36, 40, 44, 
			     48, 52, 56, 60, 64, 68, 72, 76, 80, 84, 88, 92, 
			     96,100,104,108,120,1200};
int direct_len = 30;


// RTV - reverse lookup distance to intensity
uint8_t CIdarDevice::LookupIntensity( uint8_t transmit_intensity,
				      double trans_range, 
				      bool reflection )
{
  // transmit_intensity isn't used for now

  // subtract our radius from the range reading

  trans_range -= size_x / 2.0;

  // convert range from m to inches (yuk!)
  int irange = (int)(trans_range * 39.37); // m to inches
   
  uint8_t result = 255; // make sure we got a result

  int* dist_table = 0;
  int* sigi_table = 0;
  int table_len = 0;

  // choose the tables to work from
  if( reflection )
    {
      dist_table = reflection_range;
      sigi_table = reflection_intensity;
      table_len = reflection_len;
    }
  else
    {
      dist_table = direct_range;
      sigi_table = direct_intensity;
      table_len = direct_len;
    }      

  // now do the lookup

  // if we're below the minimum distance, return the maximum intensity
  if( irange < dist_table[0] )
    {
      //printf( "vrange %d below minimum %d\n", vrange, dist_table[0] );
      result = sigi_table[0];
    }
  // if we're above the max distance, return zero intensity
  else if( irange > dist_table[ table_len-1 ] )
    {
      //printf( "irange %d is above maximum %d %s\n", 
      //      irange, dist_table[ table_len-1],
      //      reflection ? "reflect" : "direct" );
      
      result = 0;//sigi_table[ table_len-1 ];
    }
  // otherwise, interpolate a result from the table
  else
    for (int j = 1; j < table_len; j++)	
      {
	if( irange == dist_table[j] )
	  {
	    result = sigi_table[j];
	    break;
	  }
	else if (irange < dist_table[j])	
	  {
	    //printf( "irange %d is between %d and %d %s\n",
	    //    irange, dist_table[j-1], dist_table[j],
	    //    reflection ? "reflect" : "direct" );
	    
	    //# we're in the right range; calc ans;
	    result =  (uint8_t)((irange - dist_table[j-1])
				* (sigi_table[j] - sigi_table[j-1])
				/ (dist_table[j] - dist_table[j-1])
				+ sigi_table[j-1] );
	    break;
	  }
      }
  
  
  assert( result != 255 );
  
  if( result > 200 )
    printf( "WARNING: intensity suspicious!" );
  
  //printf( "RANGE: %umm %u' %uv  INT: %u\n", 
  //  trans_range, irange, vrange, result );
  
  return result;
} 



#ifdef INCLUDE_RTK2

///////////////////////////////////////////////////////////////////////////
// Initialise the rtk gui
void CIdarDevice::RtkStartup()
{
  CPlayerEntity::RtkStartup();
  
  // Create a figure representing this object
  this->data_fig = rtk_fig_create(m_world->canvas, this->fig, 49);
  this->rays_fig = rtk_fig_create(m_world->canvas, this->fig, 48);
  
  rtk_fig_origin(this->data_fig, 0,0,0 );
  rtk_fig_origin(this->rays_fig, 0,0,0 );
 
  // show our orientation in the body figure
  rtk_fig_line( this->fig, 0,0, size_x/2.0, 0);

  // Set the color
  rtk_fig_color_rgb32(this->data_fig, this->color);
  rtk_fig_color_rgb32(this->rays_fig, RGB(200,200,200) );
  
#ifdef RENDER_INDEX
  // render our index number
  char buf[16];
  sprintf( buf,"%d", m_player.index );
  rtk_fig_text(this->fig, 2.0 * size_x, 0, 0, buf );
#endif

}


///////////////////////////////////////////////////////////////////////////
// Finalise the rtk gui
void CIdarDevice::RtkShutdown()
{
  // Clean up the figure we created
  if(this->data_fig) rtk_fig_destroy(this->data_fig);
  if(this->rays_fig) rtk_fig_destroy(this->rays_fig);
  
  CPlayerEntity::RtkShutdown();
} 


///////////////////////////////////////////////////////////////////////////
// Update the rtk gui
void CIdarDevice::RtkUpdate()
{
  CPlayerEntity::RtkUpdate();
   
  // Get global pose
  //double gx, gy, gth;
  //GetGlobalPose(gx, gy, gth);
  //rtk_fig_origin(this->data_fig, gx, gy, gth );

  //  if( m_world->ShowDeviceData( this->stage_type) )
  //{
  //  rtk_fig_show( this->data_fig, true );
  //  rtk_fig_show( this->rays_fig, true );
  // }
  //else
  //{
  //  rtk_fig_show( this->data_fig, false );
  //  rtk_fig_show( this->rays_fig, false );
  // }
  
  //if( Subscribed() < 1 )
  //{
  //rtk_fig_clear( this->rays_fig );
  //rtk_fig_clear( this->data_fig );
  // }
}

#endif
