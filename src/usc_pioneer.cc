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
//  $Revision: 1.1.2.3 $
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
    m_pioneer = new CPioneerMobileDevice(world, parent, m_player);
    m_misc = new CMiscDevice(world, m_pioneer, m_player);
    m_sonar = new CSonarDevice(world, m_pioneer, m_player);
    m_laser = new CLaserDevice(world, m_pioneer, m_player);

    // Make the pioneer base the default parent of any would be children
    //
    m_default_object = m_pioneer;
    
    // Add everything into a list of anonymous objects
    //
    m_child_count = 0;
    m_child[m_child_count++] = m_player;
    m_child[m_child_count++] = m_pioneer;
    m_child[m_child_count++] = m_misc;
    m_child[m_child_count++] = m_sonar;    
    m_child[m_child_count++] = m_laser;
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
// Load the object
//
bool CUscPioneer::Load(int argc, char **argv)
{
    // Set some defaults
    //
    strcpy(m_id, "jane_doe");
    
    // Loop through args and extract settings
    //
    for (int i = 0; i < argc;)
    {
        // Extract pose
        //
        if (strcmp(argv[i], "pose") == 0 && i + 3 < argc)
        {
            double px = atof(argv[i + 1]);
            double py = atof(argv[i + 2]);
            double pth = DTOR(atof(argv[i + 3]));
            m_pioneer->SetPose(px, py, pth);
            i += 4;
        }
        
        // Extract the robot name
        //
        else if (strcmp(argv[i], "name") == 0 && i + 1 < argc)
        {
            strcpy(m_id, argv[i + 1]);
            i += 2;
        }

        // Extract player server port number
        //
        else if (strcmp(argv[i], "port") == 0 && i + 1 < argc)
        {
            m_player->m_port = atoi(argv[i + 1]);
            i += 2;
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
