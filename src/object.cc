///////////////////////////////////////////////////////////////////////////
//
// File: object.cc
// Author: Andrew Howard
// Date: 04 Dec 2000
// Desc: Base class for movable objects
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/object.cc,v $
//  $Author: ahoward $
//  $Revision: 1.1.2.10 $
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

#include <math.h>
#include "object.hh"
#include "objectfactory.hh"


///////////////////////////////////////////////////////////////////////////
// Minimal constructor
// Requires a pointer to the parent and a pointer to the world.
//
CObject::CObject(CWorld *world, CObject *parent)
{
    m_world = world;
    m_parent = parent;

    if (m_parent)
        m_depth = m_parent->m_depth + 1;
    else
        m_depth = 0;
       
    m_id[0] = 0;    
    m_lx = m_ly = m_lth = 0;

#ifdef INCLUDE_RTK
    m_draggable = false; 
    m_mouse_radius = 0;
    m_mouse_ready = false;
    m_dragging = false;
#endif
}


///////////////////////////////////////////////////////////////////////////
// Destructor
//
CObject::~CObject()
{
}


///////////////////////////////////////////////////////////////////////////
// Initialise the object from an argument list
//
bool CObject::init(int argc, char **argv)
{
    return true;
}


///////////////////////////////////////////////////////////////////////////
// Save the object back to an argument list
//
bool CObject::Save(char *buffer, size_t bufflen)
{
    return true;
}


///////////////////////////////////////////////////////////////////////////
// Startup routine
//
bool CObject::Startup()
{
    return true;
}

///////////////////////////////////////////////////////////////////////////
// Shutdown routine
//
void CObject::Shutdown()
{
}


///////////////////////////////////////////////////////////////////////////
// Update the object's representation
//
void CObject::Update()
{
}


///////////////////////////////////////////////////////////////////////////
// Convert local to global coords
//
void CObject::LocalToGlobal(double &px, double &py, double &pth)
{
    // Get the pose of our origin wrt global cs
    //
    double ox, oy, oth;
    GetGlobalPose(ox, oy, oth);

    // Compute pose based on the parent's pose
    //
    double sx = ox + px * cos(oth) - py * sin(oth);
    double sy = oy + px * sin(oth) + py * cos(oth);
    double sth = oth + pth;

    px = sx;
    py = sy;
    pth = sth;
}


///////////////////////////////////////////////////////////////////////////
// Convert global to local coords
//
void CObject::GlobalToLocal(double &px, double &py, double &pth)
{
    // Get the pose of our origin wrt global cs
    //
    double ox, oy, oth;
    GetGlobalPose(ox, oy, oth);

    // Compute pose based on the parent's pose
    //
    double sx =  (px - ox) * cos(oth) + (py - oy) * sin(oth);
    double sy = -(px - ox) * sin(oth) + (py - oy) * cos(oth);
    double sth = pth - oth;

    px = sx;
    py = sy;
    pth = sth;
}


///////////////////////////////////////////////////////////////////////////
// Set the objects pose in the parent cs
//
void CObject::SetPose(double px, double py, double pth)
{
    // Set our pose wrt our parent
    //
    m_lx = px;
    m_ly = py;
    m_lth = pth;
}


///////////////////////////////////////////////////////////////////////////
// Get the objects pose in the parent cs
//
void CObject::GetPose(double &px, double &py, double &pth)
{
    px = m_lx;
    py = m_ly;
    pth = m_lth;
}


///////////////////////////////////////////////////////////////////////////
// Set the objects pose in the global cs
//
void CObject::SetGlobalPose(double px, double py, double pth)
{
    // Get the pose of our parent in the global cs
    //
    double ox = 0;
    double oy = 0;
    double oth = 0;
    if (m_parent)
        m_parent->GetGlobalPose(ox, oy, oth);
    
    // Compute our pose in the local cs
    //
    m_lx =  (px - ox) * cos(oth) + (py - oy) * sin(oth);
    m_ly = -(px - ox) * sin(oth) + (py - oy) * cos(oth);
    m_lth = pth - oth;
}


///////////////////////////////////////////////////////////////////////////
// Get the objects pose in the global cs
//
void CObject::GetGlobalPose(double &px, double &py, double &pth)
{
    // Get the pose of our parent in the global cs
    //
    double ox = 0;
    double oy = 0;
    double oth = 0;
    if (m_parent)
        m_parent->GetGlobalPose(ox, oy, oth);
    
    // Compute our pose in the global cs
    //
    px = ox + m_lx * cos(oth) - m_ly * sin(oth);
    py = oy + m_lx * sin(oth) + m_ly * cos(oth);
    pth = oth + m_lth;
}


#ifdef INCLUDE_RTK


///////////////////////////////////////////////////////////////////////////
// UI property message handler
//
void CObject::OnUiProperty(RtkUiPropertyData* pData)
{
}


///////////////////////////////////////////////////////////////////////////
// Process GUI update messages
//
void CObject::OnUiUpdate(RtkUiDrawData *pData)
{
    pData->begin_section("global", "object");

    // Draw a marker to show we are being dragged
    //
    if (pData->draw_layer("focus", true))
    {
        if (m_mouse_ready && m_draggable)
        {
            if (m_mouse_ready)
                pData->set_color(RTK_RGB(128, 128, 255));
            if (m_dragging)
                pData->set_color(RTK_RGB(0, 0, 255));
            
            double ox, oy, oth;
            GetGlobalPose(ox, oy, oth);
            
            pData->ellipse(ox - m_mouse_radius, oy - m_mouse_radius,
                           ox + m_mouse_radius, oy + m_mouse_radius);
            pData->draw_text(ox + m_mouse_radius, oy + m_mouse_radius, m_id);
        }
    }

    pData->end_section();
}


///////////////////////////////////////////////////////////////////////////
// Process GUI mouse messages
//
void CObject::OnUiMouse(RtkUiMouseData *pData)
{
    pData->begin_section("global", "move");

    if (pData->use_mouse_mode("object"))
    {
        if (MouseMove(pData))
            pData->reset_button();
    }
    
    pData->end_section();
}


///////////////////////////////////////////////////////////////////////////
// Move object with the mouse
//
bool CObject::MouseMove(RtkUiMouseData *pData)
{
    // Get current pose
    //
    double px, py, pth;
    GetGlobalPose(px, py, pth);
    
    // Get the mouse position
    //
    double mx, my;
    pData->get_point(mx, my);
    double mth = pth;
    
    double dx = mx - px;
    double dy = my - py;
    double dist = sqrt(dx * dx + dy * dy);
    
    // See of we are within range
    //
    m_mouse_ready = (dist < m_mouse_radius);
    
    // If the mouse is within range and we are draggable...
    //
    if (m_mouse_ready && m_draggable)
    {
        if (pData->is_button_down())
        {
            // Drag on left
            //
            if (pData->which_button() == 1)
                m_dragging = true;
            
            // Rotate on right
            //
            else if (pData->which_button() == 3)
            {
                m_dragging = true;
                mth += M_PI / 8;
            }
        }
        else if (pData->is_button_up())
            m_dragging = false;
    }   
    
    // If we are dragging, set the pose
    //
    if (m_dragging)
        SetGlobalPose(mx, my, mth);

    return (m_mouse_ready);
}

#endif
