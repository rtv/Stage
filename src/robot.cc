/*************************************************************************
 * robot.cc - most of the action is here
 * RTV
 * $Id: robot.cc,v 1.13.2.1 2000-12-05 23:17:34 ahoward Exp $
 ************************************************************************/

#include <errno.h>
#include <fcntl.h>
#include <iomanip.h>
#include <iostream.h>
#include <math.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strstream.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <iomanip.h>
#include <sys/mman.h>

// BPG
#include <signal.h>
#include <sys/wait.h>
// GPB

#include <offsets.h> // from Player's include directory

#define ENABLE_TRACE 1

#include "world.h"
#include "win.h"
#include "ports.h"

// Active (player) devices
//
#include "pioneermobiledevice.hh"
#include "sonardevice.h"
#include "laserdevice.hh"
#include "visiondevice.hh"
#include "ptzdevice.hh"

// Passive devices
//
#include "laserbeacondevice.hh"


extern int errno;

const double TWOPI = 6.283185307;

//extern CWorld* world;
extern CWorldWin* win;

unsigned char f = 0xFF;

const int numPts = SONARSAMPLES;


CRobot::CRobot( CWorld* ww, int col, 
		float w, float l,
		float startx, float starty, float starta )
        : CObject(ww, NULL)
{
  world = ww;

    // Initial pose
    //
    SetPose(startx, starty, starta);
  
  //id = iid;
  color = col;
  next  = NULL;
  
  xorigin = oldx = x = startx * world->ppm;
  yorigin = oldy = y = starty * world->ppm;
  aorigin = olda = a = starta;

  channel = 0; // vision system color channel - default 0
  
  showDeviceDetail = false; // bool controls GUI display of device data/status
 
  // Initialise the device list
  //
  m_device_count = 0;
}

CRobot::~CRobot( void )
{
}


///////////////////////////////////////////////////////////////////////////
// Start all the devices
//
bool CRobot::Startup()
{
    TRACE0("starting devices");

    // Startup player
    // This will generate the memory map, so it must be done first
    //
    if (!StartupPlayer())
        return false;

    // Create pioneer device    
    //
    AddChild(new CPioneerMobileDevice(this, 
                                      world->pioneerWidth, 
                                      world->pioneerLength,
                                      playerIO + SPOSITION_DATA_START,
                                      SPOSITION_DATA_BUFFER_SIZE,
                                      SPOSITION_COMMAND_BUFFER_SIZE,
                                      SPOSITION_CONFIG_BUFFER_SIZE));

    // Create laser device
    //
    AddChild(new CLaserDevice(this, playerIO + LASER_DATA_START,
                              LASER_DATA_BUFFER_SIZE,
                              LASER_COMMAND_BUFFER_SIZE,
                              LASER_CONFIG_BUFFER_SIZE));

    // Start any child objects
    //
    CObject::Startup();
   
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

    // Start all the devices
    //
    for (int i = 0; i < m_device_count; i++)
    {
        if (!m_device[i]->Startup() )
        {
            perror("CRobot::Startup: failed to open device; device unavailable");
            m_device[i] = NULL;
        }
    }
    */
    
    return true;
}


///////////////////////////////////////////////////////////////////////////
// Shutdown the devices
//
void CRobot::Shutdown()
{
    TRACE0("shutting down devices");

    // Shutdown player
    //
    ShutdownPlayer();

    // Shutdown child objects
    //
    CObject::Shutdown();
    
    for (int i = 0; i < m_device_count; i++)
    {
        if (!m_device[i])
            continue;
        m_device[i]->Shutdown();
        delete m_device[i];
    }
}


///////////////////////////////////////////////////////////////////////////
// Update the robot 
//
void CRobot::Update()
{
    CObject::Update();
    
    // Update all devices
    //
    for (int i = 0; i < m_device_count; i++)
    {
        m_device[i]->Update();
    }
}


///////////////////////////////////////////////////////////////////////////
// Start player instance
//
bool CRobot::StartupPlayer()
{
    TRACE0("starting player");
    
    // -- create the memory map for IPC with Player --------------------------

    // amount of memory to reserve per robot for Player IO
    size_t areaSize = TOTAL_SHARED_MEMORY_BUFFER_SIZE;

    // make a unique temporary file for shared mem
    strcpy( tmpName, "playerIO.XXXXXX" );
    int tfd = mkstemp( tmpName );

    // make the file the right size
    if( ftruncate( tfd, areaSize ) < 0 )
    {
        perror( "Failed to set file size" );
        return false;
    }

    playerIO = (caddr_t)mmap( NULL, areaSize,
                              PROT_READ | PROT_WRITE, MAP_SHARED, tfd, (off_t) 0);
    if (playerIO == MAP_FAILED )
    {
        perror( "Failed to map memory" );
        return false;
    }

    close( tfd ); // can close fd once mapped

    //cout << "Mapped area: " << (unsigned int)playerIO << endl;

    // test the memory space
    for( int t=0; t<64; t++ ) playerIO[t] = (unsigned char)t;

    // ----------------------------------------------------------------------
    // fork off a player process to handle robot I/O
    if( (player_pid = fork()) < 0 )
    {
        cerr << "fork error creating robots" << flush;
        return false;
    }
    else
    {
        // BPG
        //if( pid == 0 ) // new child process
        // GPB
        if( player_pid == 0 ) // new child process
        {
            // create player port number for command line
            char portBuf[32];
            sprintf( portBuf, "%d", (int) (PLAYER_BASE_PORT + color -1) );

            // BPG
            // release controlling tty so Player doesn't get signals
            setpgrp();
            // GPB

            // we assume Player is in the current path
            if( execlp( "player", "player",
                        "-gp", portBuf, "-stage", tmpName, (char*) 0) < 0 )
            {
                cerr << "execlp failed: make sure Player can be found"
                    " in the current path."
                     << endl;
                return false;
            }
        }
        //else
        //printf("forked player with pid: %d\n", player_pid);
    }
    return true;
}


///////////////////////////////////////////////////////////////////////////
// Stop player instance
//
void CRobot::ShutdownPlayer()
{
    TRACE0("stopping player");
    
    // BPG
    if(kill(player_pid,SIGINT))
        perror("CRobot::~CRobot(): kill() failed sending SIGINT to Player");
    if(waitpid(player_pid,NULL,0) == -1)
        perror("CRobot::~CRobot(): waitpid() returned an error");
    // GPB

    // delete the playerIO.xxxxxx file
    remove( tmpName );
}


bool CRobot::HasMoved( void )
{
    // *** WARNING -- I lost something in the merge here -- ahoward
  return true; // this is a safe default for now - RTV
}

void CRobot::MapUnDraw()
{
  for (int i = 0; i < m_device_count; i++) m_device[i]->MapUnDraw();
}  


void CRobot::MapDraw()
{
  for (int i = 0; i < m_device_count; i++) m_device[i]->MapDraw();
}  

void CRobot::GUIDraw()
{
  for (int i = 0; i < m_device_count; i++) m_device[i]->GUIDraw();
}  

void CRobot::GUIUnDraw()
{
  for (int i = 0; i < m_device_count; i++) m_device[i]->GUIUnDraw();
}  


#ifdef INCLUDE_RTK

///////////////////////////////////////////////////////////////////////////
// Process GUI update messages
//
void CRobot::OnUiUpdate(RtkUiDrawData *pData)
{
    // Draw our children
    //
    CObject::OnUiUpdate(pData);
}


///////////////////////////////////////////////////////////////////////////
// Process GUI mouse messages
//
void CRobot::OnUiMouse(RtkUiMouseData *pData)
{
    CObject::OnUiMouse(pData);
}

#endif
