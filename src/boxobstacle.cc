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

#define ENABLE_RTK_TRACE 1

#include "world.hh"
#include "tokenize.hh"
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
bool CBoxObstacle::Load(char *buffer, int bufflen)
{
    // Tokenize the buffer
    //
    char *argv[64];
    int argc = Tokenize(buffer, bufflen, argv, ARRAYSIZE(argv)); 

    for (int i = 0; i < argc; )
    {
        // Ignore create token
        //
        if (strcmp(argv[i], "create") == 0 && i + 1 < argc)
            i += 2;
        
        // Extact box pose
        //
        else if (strcmp(argv[i], "pose") == 0 && i + 3 < argc)
        {
            double px = atof(argv[i + 1]);
            double py = atof(argv[i + 2]);
            double pth = DTOR(atof(argv[i + 3]));
            SetPose(px, py, pth);
            i += 4;
        }

        // Extract box size
        //
        else if (strcmp(argv[i], "size") == 0 && i + 2 < argc)
        {
            m_size_x = atof(argv[i + 1]);
            m_size_y = atof(argv[i + 2]);
            i += 3;
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
        m_draggable = (m_parent_object == NULL);
    #endif
        
    return true;
}


///////////////////////////////////////////////////////////////////////////
// Save the object to a buffer
//
bool CBoxObstacle::Save(char *buffer, int bufflen)
{
    double px, py, pth;
    GetPose(px, py, pth);
    
    snprintf(buffer, bufflen,
             "create box_obstacle pose %.2f %.2f %.2f size %.2f %.2f\n",
             (double) px, (double) py, (double) RTOD(pth),
             (double) m_size_x, (double) m_size_y);
    
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



