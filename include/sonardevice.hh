///////////////////////////////////////////////////////////////////////////
//
// File: sonardevice.hh
// Author: Richard Vaughan, Andrew Howard
// Date: 30 Nov 2000
// Desc: Simulates the pioneer sonars
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/include/sonardevice.hh,v $
//  $Author: rtv $
//  $Revision: 1.8 $
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

#ifndef SONARDEVICE_HH
#define SONARDEVICE_HH

#include "playerdevice.hh"

#define SONARSAMPLES PLAYER_NUM_SONAR_SAMPLES

enum SonarReturn { SonarTransparent=0, SonarOpaque };

class CSonarDevice : public CEntity
{
    // Default constructor
    //
    public: CSonarDevice(CWorld *world, CEntity *parent );
    
    // Update the device
    //
    public: virtual void Update( double sim_time );

    // Get the pose of the sonar
    //
    private: void GetSonarPose(int s, double &px, double &py, double &pth);
    
    // Maximum range of sonar in meters
    //
    private: double m_min_range;
    private: double m_max_range;

    // Array holding the sonar poses
    //
    private: int m_sonar_count;
    private: double m_sonar[SONARSAMPLES][3];
    
    // Array holding the sonar data
    //
    private: player_sonar_data_t m_data;


#ifdef INCLUDE_RTK2

  // Initialise the rtk gui
  protected: virtual void RtkStartup();

  // Finalise the rtk gui
  protected: virtual void RtkShutdown();

  // Update the rtk gui
  protected: virtual void RtkUpdate();
  
  // For drawing the sonar beams
  private: rtk_fig_t *scan_fig;

  // sonar scan lines end point   
   private: double hits[SONARSAMPLES][2][2];
#endif


};

#endif

