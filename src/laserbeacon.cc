///////////////////////////////////////////////////////////////////////////
//
// File: laserbeacon.cc
// Author: Andrew Howard
// Date: 4 Dec 2000
// Desc: Simulates the Player CLaserDevice (the SICK laser)
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/laserbeacon.cc,v $
//  $Author: ahoward $
//  $Revision: 1.1.2.4 $
//
// Usage:
//  This object acts a both a simple laser reflector and a more complex
//  laser beacon with an id.
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

#define ENABLE_RTK_TRACE 1

#include "world.hh"
#include "laserbeacon.hh"


///////////////////////////////////////////////////////////////////////////
// Default constructor
//
CLaserBeacon::CLaserBeacon(CWorld *world, CObject *parent)
        : CObject(world, parent)
{
    m_beacon_id = 0;
    
    // Set the initial map pose
    //
    m_map_px = m_map_py = m_map_pth = 0;
}


///////////////////////////////////////////////////////////////////////////
// Initialise the object from an argument list
//
bool CLaserBeacon::Init(int argc, char **argv)
{
    if (!CObject::Init(argc, argv))
        return false;

    for (int arg = 0; arg < argc; )
    {
        // Extact pose
        //
        if (strcmp(argv[arg], "pose") == 0 && arg + 3 < argc)
        {
            double px = atof(argv[arg + 1]);
            double py = atof(argv[arg + 2]);
            double pth = DTOR(atof(argv[arg + 3]));
            SetPose(px, py, pth);
            arg += 4;
        }

        // Extract id
        //
        else if (strcmp(argv[arg], "id") == 0 && arg + 1 < argc)
        {
            m_beacon_id = atoi(argv[argc + 1]);
            arg += 2;
        }

        // Print syntax
        //
        else
        {
            printf("syntax: create laser_beacon pose <x> <y> <th> id <id>\n");
            return false;
        }
    }
        
    #ifdef INCLUDE_RTK
        m_mouse_radius = (m_parent == NULL ? 0.2 : 0.0);
        m_draggable = (m_parent == NULL);
    #endif
        
    return true;
}

///////////////////////////////////////////////////////////////////////////
// Startup routine
//
bool CLaserBeacon::Startup()
{
    return CObject::Startup();
}


///////////////////////////////////////////////////////////////////////////
// Update the laser data
//
void CLaserBeacon::Update()
{
    //RTK_TRACE0("updating laser beacon");
    ASSERT(m_world != NULL);

    // Undraw our old representation
    //
    m_world->SetCell(m_map_px, m_map_py, layer_laser, 0);
    m_world->SetCell(m_map_px, m_map_py, layer_beacon, 0);
    
    // Update our global pose
    //
    GetGlobalPose(m_map_px, m_map_py, m_map_pth);
    
    // Draw our new representation
    //
    m_world->SetCell(m_map_px, m_map_py, layer_laser, 2);
    m_world->SetCell(m_map_px, m_map_py, layer_beacon, m_beacon_id);
}


#ifdef INCLUDE_RTK

///////////////////////////////////////////////////////////////////////////
// Process GUI update messages
//
void CLaserBeacon::OnUiUpdate(RtkUiDrawData *pData)
{
    CObject::OnUiUpdate(pData);

    pData->BeginSection("global", "laserbeacons");
    
    if (pData->DrawLayer("", true))
    {
        double r = 0.05;
        double ox, oy, oth;
        GetGlobalPose(ox, oy, oth);
        pData->SetColor(RTK_RGB(0, 0, 255));
        pData->Rectangle(ox - r, oy - r, ox + r, oy + r);
    }

    pData->EndSection();
}


///////////////////////////////////////////////////////////////////////////
// Process GUI mouse messages
//
void CLaserBeacon::OnUiMouse(RtkUiMouseData *pData)
{
    CObject::OnUiMouse(pData);
}

#endif





