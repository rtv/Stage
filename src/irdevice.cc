///////////////////////////////////////////////////////////////////////////
//
// File: sonardevice.cc
// Author: Richard Vaughan, Andrew Howard
// Date: 30 Nov 2000
// Desc: Simulates the pioneer sonars
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/irdevice.cc,v $
//  $Author: ahoward $
//  $Revision: 1.1.2.1 $
//
// Usage:
//  (empty)
//
// Theory of operation:
//  (empty)
//
// Known bugs:
//  (empty)
//
// Possible enhancements:
//  (empty)
//
///////////////////////////////////////////////////////////////////////////
#include <iostream.h>
#include <math.h>
#include "world.hh"
#include "irdevice.hh"
#include "raytrace.hh"

// constructor

CIDARDevice::CIDARDevice(CWorld *world, CEntity *parent )
  : CEntity(world, parent )
{
  puts( "IDAR" );

  m_stage_type = IDARType;

  // we're invisible except to other IDARs
  laser_return = LaserNothing;
  sonar_return = false;
  obstacle_return = false;
  idar_return = IDAR_TRANSMIT;

  // set the Player IO sizes correctly for this type of Entity
  m_data_len    = sizeof( player_idar_data_t );
  m_command_len = sizeof( player_idar_command_t );
  m_config_len  = 0;//sizeof( player_ir_config_t );
  
  m_player_type = PLAYER_IDAR_CODE; // from player's messages.h
    
  m_color_desc = IDAR_COLOR;

  m_max_range = 2.0;
    
  m_size_x = 0.1;
  m_size_y = 0.1;

  // zero the local buffers  
  memset( &m_mesg_rx, 0, m_data_len ); 
  memset( &m_mesg_tx, 0, m_command_len );
}


void CIDARDevice::Update( double sim_time ) 
{
#ifdef DEBUG
  CEntity::Update( sim_time ); // inherit some debug output
#endif

  // dump out if noone is subscribed
  if(!Subscribed())
    return;

  // Check to see if it is time to update
  //  - if not, return right away.
  if( sim_time - m_last_update < m_interval) return;
  
  m_last_update = sim_time;
  
  // Get configs  
  // this copies messages into our transmit buffer
  // where other devices can see them when they probe us
  if( !GetCommand( &m_mesg_tx, m_command_len ) )
    {
      puts( "Stage: warning - irdevice::GetConfig failed" );
    }
  

  // for now we do a 1-ray trace for each sensor - i'll replace this with
  // a chord scan soon

  double ox, oy, oth;
  GetGlobalPose( ox, oy, oth ); // start position for the ray trace

  // Do each sensor
  for (int s = 0; s < PLAYER_NUM_IDAR_SAMPLES; s++)
    {
      // Compute parameters of scan line
      double range = m_max_range;
      
      double bearing = oth + ( s * TWOPI/PLAYER_NUM_IDAR_SAMPLES );

      CLineIterator lit( ox, oy, bearing, range, 
                         m_world->ppm, m_world->matrix, PointToBearingRange );
      CEntity* ent;
      
      while( (ent = lit.GetNextEntity()) )
      {
          if( ent != this && ent != m_parent_object && ent->idar_return ) 
          {
	    // if it's a reflector, get the value i'm sending that way
	    if( ent->idar_return == IDAR_REFLECT )
	      {
		m_mesg_rx.ranges[s] = (uint16_t)(1000.0 * lit.GetRange());
		m_mesg_rx.values[s] = m_mesg_tx.values[s];
	      }

	    // if it's a transmitter, get the value the other guy sent
	    if( ent->idar_return == IDAR_TRANSMIT )
	      {
		m_mesg_rx.ranges[s] = (uint16_t)(1000.0 * lit.GetRange());
		m_mesg_rx.values[s] = ((CIDARDevice*)ent)->m_mesg_tx.values[s];
	      }		

              break;
          }
      } 
    }
  
  PutData( &m_mesg_rx, m_data_len );
  
  return;
}


