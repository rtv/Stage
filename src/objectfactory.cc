///////////////////////////////////////////////////////////////////////////
//
// File: createobject.cc
// Author: Andrew Howard
// Date: 05 Dec 2000
// Desc: Naked C function for creating different types of objects
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/objectfactory.cc,v $
//  $Author: ahoward $
//  $Revision: 1.1.2.4 $
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

#include "objectfactory.hh"
#include "laserbeaconobject.hh"
#include "visionbeacon.hh"

// Player objects
//
#include "playerrobot.hh"
#include "pioneermobiledevice.hh"
#include "laserdevice.hh"
#include "visiondevice.hh"


/////////////////////////////////////////////////////////////////////////
// Useful macros

#define FIND_PLAYER_ROBOT() CPlayerRobot *robot = (CPlayerRobot*) \
                            parent->FindAncestor(typeid(CPlayerRobot)); \
                            if (robot == NULL) \
                            { \
                                MSG("player device does not have player robot as ancestor;" \
                                    "device ignored"); \
                                return NULL; \
                            } 

/////////////////////////////////////////////////////////////////////////
// Create an object given a type
// Additional arguments can be passed in through argv.
//
CObject* CreateObject(const char *type, CWorld *world, CObject *parent)
{
    // Create laser beacon
    //
    if (strcmp(type, "laser_beacon") == 0)
        return new CLaserBeaconObject(world, parent);

    // Create vision beacon
    //
    if (strcmp(type, "vision_beacon") == 0)
        return new CVisionBeacon(world, parent);

    // Create player robot
    //
    if (strcmp(type, "player_robot") == 0)
        return new CPlayerRobot(world, parent);
        
    // Create pioneer device
    //
    if (strcmp(type, "pioneer") == 0)
    {
        FIND_PLAYER_ROBOT();
        return new CPioneerMobileDevice(world, parent, robot,
                                        robot->playerIO + SPOSITION_DATA_START,
                                        SPOSITION_TOTAL_BUFFER_SIZE);
    }
    
    // Create laser device
    //
    if (strcmp(type, "laser") == 0)
    {
        FIND_PLAYER_ROBOT();
        return new CLaserDevice(world, parent, robot,
                                robot->playerIO + LASER_DATA_START,
                                LASER_TOTAL_BUFFER_SIZE);
    }

    // Create vision device
    //
    if (strcmp(type, "vision") == 0)
    {
        FIND_PLAYER_ROBOT();
        return new CVisionDevice(world, parent, robot, NULL,
                                robot->playerIO + ACTS_DATA_START,
                                ACTS_TOTAL_BUFFER_SIZE);
    }
    
    return NULL;
}


    /* *** REMOVE ahoward
    m_device_count = 0;

    // Create pioneer device    
    //
    ASSERT_INDEX(m_device_count, m_device);
    m_device[m_device_count++] = new CPioneerMobileDevice( this, 
				world->pioneerWidth, 
				world->pioneerLength,
				playerIO + SPOSITION_DATA_START,
				SPOSITION_DATA_BUFFER_SIZE,
				SPOSITION_COMMAND_BUFFER_SIZE,
				SPOSITION_CONFIG_BUFFER_SIZE);

    // Sonar device
    //
    m_device[m_device_count++] = new CSonarDevice( this,
                                                   playerIO + SSONAR_DATA_START,
                                                   SSONAR_DATA_BUFFER_SIZE,
                                                   SSONAR_COMMAND_BUFFER_SIZE,
                                                   SSONAR_CONFIG_BUFFER_SIZE);
    
    // Create laser device
    //
    ASSERT_INDEX(m_device_count, m_device);
    m_device[m_device_count++] = new CLaserDevice(this, playerIO + LASER_DATA_START,
                                                  LASER_DATA_BUFFER_SIZE,
                                                  LASER_COMMAND_BUFFER_SIZE,
                                                  LASER_CONFIG_BUFFER_SIZE);

    // Create ptz device
    //
    CPtzDevice *ptz_device = new CPtzDevice(this, playerIO + PTZ_DATA_START,
                                            PTZ_DATA_BUFFER_SIZE,
                                            PTZ_COMMAND_BUFFER_SIZE,
                                            PTZ_CONFIG_BUFFER_SIZE);
    ASSERT_INDEX(m_device_count, m_device);
    m_device[m_device_count++] = ptz_device;

    // Create vision device
    // **** HACK -- pass a pointer to the ptz device so we can determine
    // the vision parameters.
    // A better way to do this would be to generalize the concept of a coordinate
    // frame, so that the PTZ device defines a new cs for the camera.
    //
    ASSERT_INDEX(m_device_count, m_device);
    m_device[m_device_count++] = new CVisionDevice(this, ptz_device,
                                                   playerIO + ACTS_DATA_START,
                                                   ACTS_DATA_BUFFER_SIZE,
                                                   ACTS_COMMAND_BUFFER_SIZE,
                                                   ACTS_CONFIG_BUFFER_SIZE);
    */
