///////////////////////////////////////////////////////////////////////////
//
// File: usc_pioneer.cc
// Author: Andrew Howard
// Date: 5 Mar 2000
// Desc: Class representing a standard USC pioneer robot
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/usc_pioneer.cc,v $
//  $Author: ahoward $
//  $Revision: 1.1.2.2 $
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

#include "world.hh"
#include "usc_pioneer.hh"
#include "playerrobot.hh"
#include "pioneermobiledevice.hh"
#include "miscdevice.hh"
#include "sonardevice.hh"
#include "laserdevice.hh"


///////////////////////////////////////////////////////////////////////////
// Minimal constructor
//
CUscPioneer::CUscPioneer(CWorld *world, CObject *parent)
        : CObject(world, parent)
{
    m_player = new CPlayerRobot(world, this);
    m_pioneer = new CPioneerMobileDevice(world, this, m_player);
    m_misc = new CMiscDevice(world, m_pioneer, m_player);
    m_sonar = new CSonarDevice(world, m_pioneer, m_player);
    m_laser = new CLaserDevice(world, m_pioneer, m_player);
    
    // Add everything into a list of anonymous objects
    //
    m_child_count = 0;
    m_child[m_child_count++] = m_player;
    m_child[m_child_count++] = m_pioneer;
    m_child[m_child_count++] = m_misc;
    m_child[m_child_count++] = m_sonar;    
    m_child[m_child_count++] = m_laser;

    #ifdef INCLUDE_RTK
    m_mouse_radius = 0.6;
    m_draggable = true;
    #endif
}


///////////////////////////////////////////////////////////////////////////
// Destructor
//
CUscPioneer::~CUscPioneer()
{
    for (int i = m_child_count - 1; i >= 0; i--)
        delete m_child[i];
}


///////////////////////////////////////////////////////////////////////////
// Initialise the object from an argument list
//
bool CUscPioneer::init(int argc, char **argv)
{
    if (!CObject::init(argc, argv))
        return false;

    // Set some defaults
    //
    strcpy(m_id, "jane_doe");
    
    // Loop through args and extract settings
    //
    for (int arg = 0; arg < argc;)
    {
        // Extact robot pose
        //
        if (strcmp(argv[arg], "pose") == 0 && arg + 3 < argc)
        {
            double px = atof(argv[arg + 1]);
            double py = atof(argv[arg + 2]);
            double pth = DTOR(atof(argv[arg + 3]));
            SetPose(px, py, pth);
            arg += 4;
        }
        
        // Extract the robot name
        //
        else if (strcmp(argv[arg], "name") == 0 && arg + 1 < argc)
        {
            strcpy(m_id, argv[arg + 1]);
            arg += 2;
        }

        // Extract player server port number
        //
        else if (strcmp(argv[arg], "port") == 0 && arg + 1 < argc)
        {
            m_player->m_port = atoi(argv[arg + 1]);
            arg += 2;
        }
        
        // Print the syntax
        //
        else
        {
            printf("syntax: create usc_pioneer pose <x> <y> <th>\n");
            printf("                           name <name> port <port>\n");
            return false;
        }
    }
    return true;
}


///////////////////////////////////////////////////////////////////////////
// Save the object back to an argument list
//
bool CUscPioneer::Save(char *buffer, size_t bufflen)
{
    double px, py, pth;
    GetPose(px, py, pth);
    
    snprintf(buffer, bufflen,
             "create usc_pioneer pose %.2f %.2f %.2f name %s port %d", 
             (double) px, (double) py, (double) RTOD(pth),
             (char*) m_id, (int) m_player->m_port);
    
    return true;
}


///////////////////////////////////////////////////////////////////////////
// Default startup -- doesnt do much
//
bool CUscPioneer::Startup()
{
    if (!CObject::Startup())
        return false;

    for (int i = 0; i < m_child_count; i++)
        m_child[i]->Startup();
    
    return true;
}


///////////////////////////////////////////////////////////////////////////
// Default shutdown -- doesnt do much
//
void CUscPioneer::Shutdown()
{
    for (int i = m_child_count - 1; i >= 0; i--)
        m_child[i]->Shutdown();
    
    CObject::Shutdown();
}


///////////////////////////////////////////////////////////////////////////
// Update robot and all its devices
//
void CUscPioneer::Update()
{
    for (int i = 0; i < m_child_count; i++)
        m_child[i]->Update();    

    CObject::Update();
}

#ifdef INCLUDE_RTK

///////////////////////////////////////////////////////////////////////////
// Process GUI update messages
//
void CUscPioneer::OnUiUpdate(RtkUiDrawData *data)
{
    for (int i = 0; i < m_child_count; i++)
        m_child[i]->OnUiUpdate(data);  
  
    CObject::OnUiUpdate(data);
}


///////////////////////////////////////////////////////////////////////////
// Process GUI mouse messages
//
void CUscPioneer::OnUiMouse(RtkUiMouseData *data)
{
    for (int i = 0; i < m_child_count; i++)
        m_child[i]->OnUiMouse(data);  
    
    CObject::OnUiMouse(data);
}


///////////////////////////////////////////////////////////////////////////
// Process GUI property messages
//
void CUscPioneer::OnUiProperty(RtkUiPropertyData *data)
{
    for (int i = 0; i < m_child_count; i++)
        m_child[i]->OnUiProperty(data);  
    
    CObject::OnUiProperty(data);
}

#endif
