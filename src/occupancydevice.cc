///////////////////////////////////////////////////////////////////////////
//
// File: occupancydevice.cc
// Author: Richard Vaughan
// Date: 7 July 2001
// Desc: publishes world background to player as an occupancy grid
//
// CVS info:
//  $Source: &
//  $Author: vaughan $
//  $Revision: 1.1 $
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

#include <stage.h>
#include "world.hh"
#include "occupancydevice.hh"

#define DEBUG

///////////////////////////////////////////////////////////////////////////
// Default constructor
//
COccupancyDevice::COccupancyDevice(CWorld *world, CEntity *parent )
        : CEntity(world, parent )
{
  // set the Player IO sizes correctly for this type of Entity
  
  // m_data_len is determined dynamically in Startup()
  m_command_len = 0;//sizeof( misc_command_buffer_t );
  m_config_len  = 0;//sizeof( misc_config_buffer_t );
  
  m_player_type = PLAYER_OCCUPANCY_CODE;
  m_player_port = GLOBALPORT; // visible to all players

  m_interval = 1.0;
}

COccupancyDevice::~COccupancyDevice( void )
{
  if( m_pixels ) delete[] m_pixels;
}

// determine how much mmapped io space we'll need for this device
// also process the world's background image and store a local
// compact representation

bool COccupancyDevice::Startup( void )
{ 
  puts( "COccupancyDevice::Startup()" );

  //count the number of filled pixels in the world's background image
  m_filled_pixels = 0;
  m_total_pixels = m_world->m_bimg->width * m_world->m_bimg->height;

  for( unsigned int index = 0; index < m_total_pixels; index++ )
    if( m_world->m_bimg->get_pixel( index) != 0 )
      m_filled_pixels++;
  

  m_data_len = sizeof( player_occupancy_data_t ) 
    + m_filled_pixels * sizeof(pixel_t);

#ifdef DEBUG
  printf( "Stage occupancy device: %lu/%lu pixels filled, require %d bytes\n", 
	  m_filled_pixels, m_total_pixels, m_data_len );
#endif
 
  // now we record data about the world's background environment
  m_width  = m_world->m_bimg->width;
  m_height = m_world->m_bimg->height;
  m_ppm    = (unsigned int)m_world->ppm;

  // allocate storage for the filled pixels
  m_pixels = new pixel_t[ m_filled_pixels ];

  memset( m_pixels, 0, m_filled_pixels * sizeof( pixel_t ) );

  int store = 0;
  // iterate through again, this time recording the pixel's details
  for( int index = 0; index < (int)m_total_pixels; index++ )
    {
      unsigned int val = m_world->m_bimg->get_pixel( index );
      
      if( val != 0 ) // it's not a background pixel
	{
	  m_pixels[store].color = val;
	  m_pixels[store].x = index % m_width;
	  m_pixels[store].y = index / m_width;

	  store++;
	}
    }

//    puts( "SCANNED PIXELS" );
//    for( int i=0; i< (int)m_filled_pixels; i++ )
//      {
//        pixel_t* p = &(m_pixels[i]);
      
//        printf( " (%d %d %d)", p->x, p->y, p->color );
//      }

//    puts( "" );
  
  return true;
}

///////////////////////////////////////////////////////////////////////////
// Update the data
//
void COccupancyDevice::Update( double sim_time )
{
#ifdef DEBUG
  CEntity::Update( sim_time ); // inherit debug output
#endif

  if( Subscribed() < 1 ) return;

  // if the interval is set to -1, we're no longer updating this device
  if( m_interval == -1 ) return;

  // Check to see if it is time to update
  //  - if not, return right away.
  if( sim_time - m_last_update < m_interval) return;
  
  m_last_update = sim_time;
  
  unsigned char* buf = new unsigned char[ m_data_len ];
 
  // put the occupancy grid header data in the start of the buffer
  player_occupancy_data_t *og = (player_occupancy_data_t*)buf;

  og->width = (uint16_t)m_width;
  og->height = (uint16_t)m_height;
  og->ppm = (uint16_t)m_ppm;
  og->num_pixels = (uint32_t)m_filled_pixels;

  // then copy the pixels after the header
  memcpy( buf + sizeof(player_occupancy_data_t), 
	  m_pixels, 
	  m_filled_pixels * sizeof( pixel_t ) );

  PutData( buf, m_data_len );
  
  // we disable this device once we've put the data once.
  m_interval = -1;
  
  delete[] buf;
}











