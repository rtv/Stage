///////////////////////////////////////////////////////////////////////////
//
// File: laserbeacondevice.hh
// Author: Andrew Howard
// Date: 12 Jan 2000
// Desc: Simulates the laser-based beacon detector
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/include/laserbeacondevice.hh,v $
//  $Author: rtv $
//  $Revision: 1.11 $
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

  // place to keep the fake lbd parameters that we'll pass back to clients
  // who ask for them
  private: char m_bit_count;
  private: short m_bit_size, m_zero_thresh, m_one_thresh;

  // this one keeps track of whether or not we've already subscribed to the
  // underlying laser device
  private: bool m_laser_subscribedp;

#ifdef INCLUDE_RTK
    
  // Process GUI update messages
  public: virtual void OnUiUpdate(RtkUiDrawData *pData);

  // Draw the beacon data
  public: void DrawData(RtkUiDrawData *event);

#endif

#ifdef INCLUDE_RTK2

  // Initialise the rtk gui
  protected: virtual void RtkStartup();

  // Finalise the rtk gui
  protected: virtual void RtkShutdown();

  // Update the rtk gui
  protected: virtual void RtkUpdate();
  
  // For drawing the sonar beams
  private: rtk_fig_t *beacon_fig;
#endif

};

#endif






