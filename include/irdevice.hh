///////////////////////////////////////////////////////////////////////////
//
// File: irdevice.hh
// Author: Richard Vaughan, Andrew Howard
// Date: 30 Nov 2000
// Desc: Simulates the HRL IR communicator/ranger
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/include/irdevice.hh,v $
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

#ifndef IDARDEVICE_HH
#define IDARDEVICE_HH

#include "entity.hh"

#define IDAR_REFLECT 1
#define IDAR_TRANSMIT 2

class CIDARDevice : public CEntity
{
  // Default constructor
  //
public: 
  CIDARDevice(CWorld *world, CEntity *parent );
  
  // these point to the data and command buffers
  player_idar_data_t m_mesg_rx;
  player_idar_command_t m_mesg_tx;
  
    // Update the device
    //
    public: virtual void Update( double sim_time );

    private: double m_max_range;
};

#endif

