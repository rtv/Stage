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
//  $Revision: 1.1.2.6 $
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
#include "object.hh"

// Forward declare object classes
//
class CPlayerRobot;
class CPioneerMobileDevice;
class CMiscDevice;
class CSonarDevice;
class CLaserDevice;
class CLaserBeaconDevice;


// Base class for all player devices
//
class CUscPioneer : public CObject
{
    // Minimal constructor
    //
    public: CUscPioneer(CWorld *world, CObject *parent);

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
    private: CPlayerRobot *m_player;
    private: CPioneerMobileDevice *m_pioneer;
    private: CMiscDevice *m_misc;
    private: CSonarDevice *m_sonar;
    private: CLaserDevice *m_laser;
    private: CLaserBeaconDevice *m_laser_beacon;

    // Anonymous list of objects/devices
    //
    private: int m_child_count;
    private: CObject *m_child[64];

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
