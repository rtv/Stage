///////////////////////////////////////////////////////////////////////////
//
// File: laserbeacondevice.hh
// Author: Andrew Howard
// Date: 12 Jan 2000
// Desc: Simulates the laser-based beacon detector
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/include/laserbeacondevice.hh,v $
//  $Author: ahoward $
//  $Revision: 1.8.4.1 $
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

// LBD = Laser Beacon Detector!

#ifndef LBDDEVICE_HH
#define LBDDEVICE_HH

#include "entity.hh"
#include "laserdevice.hh"
#include "guiexport.hh"

class CLBDDevice : public CEntity
{
  // Default constructor
  public: CLBDDevice(CWorld *world, CLaserDevice *parent );

  // Load the entity from the worldfile
  public: virtual bool Load(CWorldFile *worldfile, int section);

  // Update the device
  public: virtual void Update( double sim_time );

  // Pointer to laser used as souce of data
  private: CLaserDevice *laser;

  // Time of last update
  private: uint32_t time_sec, time_usec;

  // Detection parameters
  private: double max_range_anon;
  private: double max_range_id;
    
  private:  ExportLaserBeaconDetectorData expBeacon; 

#ifdef INCLUDE_RTK
    
  // Process GUI update messages
  public: virtual void OnUiUpdate(RtkUiDrawData *pData);

  // Draw the beacon data
  public: void DrawData(RtkUiDrawData *event);

#endif
};

#endif






