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
//  $Revision: 1.1.2.6 $
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
bool CLaserBeacon::Load(int argc, char **argv)
{
    if (!CObject::Load(argc, argv))
        return false;

    for (int i = 0; i < argc; )
    {
        // Extact beacon pose
        //
        if (strcmp(argv[i], "pose") == 0 && i + 3 < argc)
        {
            double px = atof(argv[i + 1]);
            double py = atof(argv[i + 2]);
            double pth = DTOR(atof(argv[i + 3]));
            SetPose(px, py, pth);
            i += 4;
        }

        // Extract id
        //
        else if (strcmp(argv[i], "id") == 0 && i + 1 < argc)
        {
            m_beacon_id = atoi(argv[i + 1]);
            i += 2;
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
        m_mouse_radius = (m_parent_object == NULL ? 0.2 : 0.0);
        m_draggable = (m_parent_object == NULL);
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

    pData->begin_section("global", "laserbeacons");
    
    if (pData->draw_layer("", true))
    {
        double r = 0.05;
        double ox, oy, oth;
        GetGlobalPose(ox, oy, oth);
        pData->set_color(RTK_RGB(0, 0, 255));
        pData->rectangle(ox - r, oy - r, ox + r, oy + r);
    }

    pData->end_section();
}


///////////////////////////////////////////////////////////////////////////
// Process GUI mouse messages
//
void CLaserBeacon::OnUiMouse(RtkUiMouseData *pData)
{
    CObject::OnUiMouse(pData);
}

#endif





