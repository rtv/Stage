///////////////////////////////////////////////////////////////////////////
//
// File: usc_pioneer.hh
// Author: Andrew Howard
// Date: 5 Mar 2000
// Desc: Class representing a standard USC pioneer robot
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/include/usc_pioneer.hh,v $
//  $Author: ahoward $
//  $Revision: 1.2 $
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

#ifndef USC_PIONEER_HH
#define USC_PIONEER_HH

// For base class
//
#include "entity.hh"

// Forward declare object classes so we dont need to include headers.
//
class CPlayerServer;
class CPioneerMobileDevice;
class CMiscDevice;
class CSonarDevice;
class CLaserDevice;
class CLaserBeaconDevice;
class CPtzDevice;
class CVisionDevice;


// Base class for all player devices
//
class CUscPioneer : public CEntity
{
    // Minimal constructor
    //
    public: CUscPioneer(CWorld *world, CEntity *parent);

    // Destructor
    //
    public: ~CUscPioneer();

    // Load the object from an argument list
    //
    public: virtual bool Load(int argc, char **argv);

    // Save the object to an argument list
    //
    public: virtual bool Save(int &argc, char **argv);
    
    // Initialise the device
    //
    public: virtual bool Startup();

    // Close the device
    //
    public: virtual void Shutdown();

    // Update robot and all its devices
    //
    public: virtual void Update();
    
    // Child objects/devices
    //
    private: CPlayerServer *m_player;
    private: CPioneerMobileDevice *m_pioneer;
    private: CMiscDevice *m_misc;
    private: CSonarDevice *m_sonar;
    private: CLaserDevice *m_laser;
    private: CLaserBeaconDevice *m_laser_beacon;
    private: CPtzDevice *m_ptz;
    private: CVisionDevice *m_vision;


#ifdef INCLUDE_RTK
    
    // Process GUI update messages
    //
    public: virtual void OnUiUpdate(RtkUiDrawData *data);

    // Process GUI mouse messages
    //
    public: virtual void OnUiMouse(RtkUiMouseData *data);

    // Process GUI property messages
    //
    public: virtual void OnUiProperty(RtkUiPropertyData *data);

#endif
};


#endif
