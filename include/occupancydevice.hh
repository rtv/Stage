///////////////////////////////////////////////////////////////////////////
//
// File: occupancydevice.hh
// Author: Richard Vaughan
// Date: 7 July 2001
// Desc: publishes world background data as an occupancy grid
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/include/occupancydevice.hh,v $
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

#ifndef OGDEVICE_HH
#define OGDEVICE_HH

#include "entity.hh"

class COccupancyDevice : public CEntity
{
    // constructor
    //
    public: COccupancyDevice(CWorld *world, CEntity *parent );

  // destructor
public: ~COccupancyDevice( void );
    
    // Update the device
    //
    public: virtual void Update( double sim_time );

    public: void FillOccupancyGrid( player_occupancy_data_t &og );

    public: virtual bool Startup( void );

private:
  unsigned int m_width, m_height, m_ppm;
  long unsigned int m_total_pixels, m_filled_pixels;
  pixel_t* m_pixels;
};

#endif






