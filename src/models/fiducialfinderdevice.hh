///////////////////////////////////////////////////////////////////////////
//
// File: laserbeacondevice.hh
// Author: Andrew Howard
// Date: 12 Jan 2000
// Desc: Simulates the laser-based beacon detector
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/models/fiducialfinderdevice.hh,v $
//  $Author: rtv $
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

// LBD = Laser Beacon Detector!

#ifndef LBDDEVICE_HH
#define LBDDEVICE_HH

#include "playerdevice.hh"
#include "laserdevice.hh"

#define MAXBEACONS 100

typedef struct  
{
  double x, y, th;
  int id;
} BeaconData;

typedef struct
{
  BeaconData beacons[MAXBEACONS];
  int beaconCount;
} ExportLaserBeaconDetectorData;

class CFiducialFinder : public CPlayerEntity
{
  // Default constructor
  public: CFiducialFinder( LibraryItem *libit, CWorld *world, CLaserDevice *parent );

  // a static named constructor - a pointer to this function is given
  // to the Library object and paired with a string.  When the string
  // is seen in the worldfile, this function is called to create an
  // instance of this entity
public: static CFiducialFinder* Creator(  LibraryItem *libit, CWorld *world, CEntity *parent )
  { return( new CFiducialFinder( libit, world, (CLaserDevice*)parent ) ); }
    
  // Startup routine
  public: virtual bool Startup();

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






