/******************************************************************************
* File: irdevice.cc
* Author: Richard Vaughan
* Date: 22 October 2001
* Desc: Simulates a single sensor of HRL's Infrared Data and Ranging Model
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
* $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/models/idar.cc,v $
* $Author: rtv $
* $Revision: 1.1.2.1 $
******************************************************************************/


#define DEBUG
//#undef DEBUG

#include <math.h>
#include <assert.h>

#include "sio.h"
#include "idar.hh"
#include "raytrace.hh"

// control display of player index in device body
//#define RENDER_INDEX
#define RENDER_SCANLINES
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


// constructor 
CIdarModel::CIdarModel( int id, char* token, char* color, CEntity *parent )
  : CEntity( id, token, color, parent )
{
  // we're invisible except to other IDARs
  laser_return = LaserTransparent;
  sonar_return = false;
  obstacle_return = false;
  idar_return = IDARReceive;

  max_range = IDAR_MAX_RANGE;
  min_range = 0.0;
  
  // by default we inherit the size of our parent
  if( m_parent_entity && m_parent_entity != CEntity::root )
    {
      this->size_x = m_parent_entity->size_x;
      this->size_y = m_parent_entity->size_y;
    }
  else 
    {
      size_x = 0.12; // this is the actual physical size of the HRL device
      size_y = 0.12; // but can be changed in the world file to suit
    }

  m_interval = 0.001; // updates frequently,
  // but only has work to do when it receives a command
  // so it's not a CPU hog unless we send it lots of messages
 
  m_num_scanlines = IDAR_SCANLINES; 
  m_angle_per_sensor = IDAR_TRANSMIT_ANGLE;
  m_angle_per_scanline = m_angle_per_sensor / m_num_scanlines;


  // Initialise the sonar poses to default values
  this->transducer_count = 8; // default to the HRL Pherobot arrangement
  
  // we don't need a rectangle 
  SetRects( NULL, 0 );
  
  // arrange the transducers in a circle
  for (int i = 0; i < this->transducer_count; i++)
    {
      double angle = i * TWOPI / this->transducer_count;
      this->transducers[i][0] = size_x/2.0 * cos( angle );
      this->transducers[i][1] = size_y/2.0 * sin( angle );
      this->transducers[i][2] = angle;
    }
  
}

// handle IDAR-specific properties
int CIdarModel::Property( int con, stage_prop_id_t property, 
			  void* value, size_t len, stage_buffer_t* reply )
{
  PRINT_DEBUG2( "idar ent %d prop %s", stage_id, SIOPropString(property) );
  
  switch( property )
    {
    case STG_PROP_IDAR_TXRX:
      PRINT_DEBUG( "idar TXRX" );
      
      // call TX with the send message, then RX to fill in the reply 
      this->Property( con, STG_PROP_IDAR_TX, value, len, NULL );
      this->Property( con, STG_PROP_IDAR_RX, NULL, 0, reply );

      break;
      
    case STG_PROP_IDAR_TX:
      if( value )
	{
	  PRINT_DEBUG( "idar transmitting message" );
	  TransmitMessage( (stage_idar_tx_t*)value );
	}
      if( reply ) // just ACK with no data
	SIOBufferProperty( reply, this->stage_id, STG_PROP_IDAR_TX,
			   NULL, 0, STG_ISREPLY);
      break;
      
    case STG_PROP_IDAR_RX:
      if( value )
	PRINT_WARN( "idar RX called with value - ignoring" );
      if( reply )
	{
	  PRINT_DEBUG( "idar returning best message" );
	  // reply with the contents of our data buffer
	  SIOBufferProperty( reply, this->stage_id, STG_PROP_IDAR_RX,
			     this->buffer_data.data, this->buffer_data.len,
			     STG_ISREPLY);
	  	  
	  stage_idar_rx_t blankrx;
	  memset( &blankrx, 0, sizeof(blankrx) );
	  Property( -1, STG_PROP_ENTITY_DATA, 
		    &blankrx, sizeof(blankrx), NULL );
	}
      break;


    default: 
      break;
    }
  
  // inherit base behavior
  return CEntity::Property( con, property, value, len, reply );
}


void CIdarModel::TransmitMessage( stage_idar_tx_t* transmit )
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
  

#ifdef INCLUDE_RTK2
  rtk_fig_clear(this->rays_fig);
#endif
  
  for( int tran = 0; tran < transducer_count; tran++ )
    //for( int tran = 0; tran < 1; tran++ )
    {
      double ox = transducers[tran][0];
      double oy = transducers[tran][1];
      double oth = transducers[tran][2];

      LocalToGlobal( ox, oy, oth );
      
      double scan_start_angle = 
	(oth - m_angle_per_sensor/2.0) + m_angle_per_scanline/2.0;
      
      for( int scanline=0; scanline < m_num_scanlines; scanline++ )
	{
	  
	  // Compute parameters of scan line
	  double scanline_bearing = 
	    scan_start_angle + scanline * m_angle_per_scanline;
	  
	  // normalize to make things easier to read
	  scanline_bearing = fmod( scanline_bearing, TWOPI );
	  
	  CLineIterator lit( ox, oy, scanline_bearing, max_range,
			 CEntity::matrix,
			     PointToBearingRange );
	  
	  //printf( "direction %d scanline %d "
	  //      "from %.2f,%.2f %.2fm at %.2f radians\n", 
	  //      s, scanline, ox, oy, max_range, scanline_bearing );
	  
	  CEntity* ent;
	  
	  double range = max_range;
	  
	  // get the next thing in our beam, until the beam is blocked
	  while( (ent = lit.GetNextEntity() ) ) 
	    {
	      //printf( "IDARReceive == %d\n", IDARReceive );
	      
	      // if it's not me or my parent or a sibling and it's not
	      // transparent to idar, i'll try sending a message to it
	      if( (ent != (CEntity*)this) && 
		  (ent != this->m_parent_entity) && 
		  //(ent->m_parent_entity != this->m_parent_entity) &&
		  ( ent->idar_return != IDARTransparent) )
		{		    
		  range = lit.GetRange();
		  assert( range >= 0 );
		  
		  //printf( "s:%d.%d range: %.2f\n",
		  //    s, scanline, range );
		  
		  PRINT_DEBUG2( "hit entity %d (%s)",
				ent->stage_id, ent->token );
		  
		  uint8_t intensity;
		  
		  switch( ent->idar_return )
		    {
		    case IDARReceive: // it's a receiver
		      // attempt to poke this message into his receiver
		      //printf( "TRANSMIT to %p type %s idar_return %d\n",
		      //ent, ent->token, ent->idar_return );
		      
		      
		      //PRINT_DEBUG1( "POKING A MESSAGE INTO %p", ent );
		      
		      if( (intensity = LookupIntensity(0,range,false)) > 0 )
			((CIdarModel*)ent)->
			  ReceiveMessage( (CEntity*)this, 
					  transmit->mesg,
					  transmit->len,
					  intensity,
					  false );
		      
		      // continue to next case, since IR receivers are
		      // also reflectors.
		      //break;
		      
		    case IDARReflect: // it's a reflector (ie. an obstacle)
		      //printf( "REFLECT from %p type %s idar_return %d\n",
		      //	ent, ent->token, ent->idar_return );
		      
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
	  // rays_fig is in _global_ coords
	  rtk_fig_color_rgb32(this->rays_fig, 0xAAAAAA );	
	  rtk_fig_arrow(this->rays_fig, ox, oy, scanline_bearing, range, 0.03);
#endif
#endif    
	}
    }
}


bool CIdarModel::ReceiveMessage( CEntity* sender,
				  unsigned char* mesg, int len, 
				  uint8_t intensity,
				  bool reflection )
{
  //PRINT_WARN( "RECEIVE MESSAGE" );

  //printf( "mesg recv - sensor: %d  len: %d  intensity: %d  refl: %d\n",
  //  sensor, len, intensity, reflection );
  
  stage_idar_rx_t* best = (stage_idar_rx_t*)(this->buffer_data.data);
  // TEST INTENSITY
  // we only accept this message if it's the most intense thing we've seen
  if( best && (intensity < best->intensity) )
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
  
  PRINT_DEBUG( "message accepted" );
  
  // looks good  - accept the message
  stage_idar_rx_t recv;
  memcpy( &recv.mesg, mesg, len );
  recv.len = len;
  recv.intensity = intensity;
  recv.reflection = (uint8_t)reflection;
  
  recv.timestamp_sec = (uint32_t)CEntity::simtime;
  recv.timestamp_usec = (uint32_t)(fmod(CEntity::simtime, 1.0) * MILLION);
  
  // this is the right way to export data, so everyone else finds out about it
  this->Property( -1, STG_PROP_ENTITY_DATA, 
		  &recv, sizeof(recv), NULL );
  
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
uint8_t CIdarModel::LookupIntensity( uint8_t transmit_intensity,
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
    PRINT_WARN( "intensity suspicious!" );
  
  //printf( "RANGE: %umm %u' %uv  INT: %u\n", 
  //  trans_range, irange, vrange, result );
  
  return result;
} 


