///////////////////////////////////////////////////////////////////////////
//
// File: boxobstacle.cc
// Author: Andrew Howard
// Date: 18 Dec 2000
// Desc: Simulates box obstacles
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/boxobstacle.cc,v $
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

#define ENABLE_RTK_TRACE 1

#include "world.hh"
#include "boxobstacle.hh"


///////////////////////////////////////////////////////////////////////////
// Default constructor
//
CBoxObstacle::CBoxObstacle(CWorld *world, CObject *parent)
        : CObject(world, parent)
{
    m_size_x = 1;
    m_size_y = 1;
        
    // Set the initial map pose
    //
    m_map_px = m_map_py = m_map_pth = 0;
}


///////////////////////////////////////////////////////////////////////////
// Initialise the object from an argument list
//
bool CBoxObstacle::init(int argc, char **argv)
{
    if (!CObject::init(argc, argv))
        return false;

    for (int arg = 0; arg < argc; )
    {
        // Extact box pose
        //
        if (strcmp(argv[arg], "pose") == 0 && arg + 3 < argc)
        {
            double px = atof(argv[arg + 1]);
            double py = atof(argv[arg + 2]);
            double pth = DTOR(atof(argv[arg + 3]));
            SetPose(px, py, pth);
            arg += 4;
        }

        // Extract box size
        //
        else if (strcmp(argv[arg], "size") == 0 && arg + 2 < argc)
        {
            m_size_x = atof(argv[arg + 1]);
            m_size_y = atof(argv[arg + 2]);
            arg += 3;
        }

        // Print syntax
        //
        else
        {
            printf("syntax: create box_obstacle pose <x> <y> <th> size <dx> <dy>\n");
            return false;
        }
    }

    #ifdef INCLUDE_RTK
        m_mouse_radius = max(m_size_x, m_size_y) * 0.6;
        m_draggable = (m_parent == NULL);
    #endif
        
    return true;
}


///////////////////////////////////////////////////////////////////////////
// Update the laser data
//
void CBoxObstacle::Update()
{
    //RTK_TRACE0("updating laser beacon");
    ASSERT(m_world != NULL);

    // Undraw our old representation
    //
    m_world->SetRectangle(m_map_px, m_map_py, m_map_pth,
                          m_size_x, m_size_y, layer_obstacle, 0);
    m_world->SetRectangle(m_map_px, m_map_py, m_map_pth,
                          m_size_x, m_size_y, layer_laser, 0);
    
    // Update our global pose
    //
    GetGlobalPose(m_map_px, m_map_py, m_map_pth);
    
    // Draw our new representation
    //
    m_world->SetRectangle(m_map_px, m_map_py, m_map_pth,
                          m_size_x, m_size_y, layer_obstacle, 1);
    m_world->SetRectangle(m_map_px, m_map_py, m_map_pth,
                          m_size_x, m_size_y, layer_laser, 1);

}


#ifdef INCLUDE_RTK

///////////////////////////////////////////////////////////////////////////
// Process GUI update messages
//
void CBoxObstacle::OnUiUpdate(RtkUiDrawData *pData)
{
    CObject::OnUiUpdate(pData);

    pData->begin_section("global", "BoxObstacles");
    
    if (pData->draw_layer("", TRUE))
    {
        double ox, oy, oth;
        GetGlobalPose(ox, oy, oth);
        pData->set_color(RTK_RGB(128, 128, 255));
        pData->ex_rectangle(ox, oy, oth, m_size_x, m_size_y);
    }

    pData->end_section();
}


///////////////////////////////////////////////////////////////////////////
// Process GUI mouse messages
//
void CBoxObstacle::OnUiMouse(RtkUiMouseData *pData)
{
    CObject::OnUiMouse(pData);
}

#endif



