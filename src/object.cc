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
//  $Revision: 1.1.2.1 $
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

#include <math.h>
#include "object.hh"



///////////////////////////////////////////////////////////////////////////
// Minimal constructor
// Requires a pointer to the parent and a pointer to the world.
//
CObject::CObject(CWorld *world, CObject *parent)
{
    m_world = world;
    m_parent = parent;
    m_child_count = 0;

    m_lx = m_ly = m_lth = 0;
    m_gx = m_gy = m_gth = 0;
    m_global_dirty = true;

#ifdef INCLUDE_RTK
    m_dragging = false;
    m_drag_radius = 0.1;
#endif
}


///////////////////////////////////////////////////////////////////////////
// Destructor
// Will delete children
//
CObject::~CObject()
{
    for (int i = 0; i < m_child_count; i++)
        delete m_child[i];
}


///////////////////////////////////////////////////////////////////////////
// Startup routine -- creates objects in the world
//
bool CObject::Startup()
{
    for (int i = 0; i < m_child_count; i++)
    {
        if (!m_child[i]->Startup())
            return false;
    }
    return true;
}


///////////////////////////////////////////////////////////////////////////
// Shutdown routine -- deletes objects in the world
//
void CObject::Shutdown()
{
    for (int i = 0; i < m_child_count; i++)
        m_child[i]->Shutdown();
}


///////////////////////////////////////////////////////////////////////////
// Update the object's representation
//
void CObject::Update()
{
    for (int i = 0; i < m_child_count; i++)
        m_child[i]->Update();
}


/////////////////////////////////////////////////////////////////////////
// Add a child object
//
void CObject::AddChild(CObject *child)
{
    ASSERT_INDEX(m_child_count, m_child);
    m_child[m_child_count++] = child;
}


///////////////////////////////////////////////////////////////////////////
// Convert local to global coords
//
void CObject::LocalToGlobal(double &px, double &py, double &pth)
{
    // Get the parent's global pose
    //
    double ox = 0;
    double oy = 0;
    double oth = 0;
    if (m_parent)
        m_parent->GetGlobalPose(ox, oy, oth);

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
// Set the objects pose in the parent cs
//
void CObject::SetPose(double px, double py, double pth)
{
    m_lx = px;
    m_ly = py;
    m_lth = pth;

    // Mark ourselves and all our childen as dirty
    //
    SetGlobalDirty();
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
// Get the objects pose in the global cs
//
void CObject::SetGlobalPose(double px, double py, double pth)
{
    // *** HACK not yet implemented
    ASSERT(false);
}


///////////////////////////////////////////////////////////////////////////
// Get the objects pose in the global cs
//
void CObject::GetGlobalPose(double &px, double &py, double &pth)
{
    // If the global pose is out-of-date, recompute it
    //
    if (m_global_dirty)
    {
        double ox = m_lx;
        double oy = m_ly;
        double oth = m_lth;
        LocalToGlobal(ox, oy, oth);

        m_gx = ox;
        m_gy = oy;
        m_gth = oth;
        m_global_dirty = false;
    }

    px = m_gx;
    py = m_gy;
    pth = m_gth;
}

/////////////////////////////////////////////////////////////////////////
// Mark ourselves and all our children as dirty
//
void CObject::SetGlobalDirty()
{
    m_global_dirty = true;

    for (int i = 0; i < m_child_count; i++)
        m_child[i]->SetGlobalDirty();
}

#ifdef INCLUDE_RTK

///////////////////////////////////////////////////////////////////////////
// Process GUI update messages
//
void CObject::OnUiUpdate(RtkUiDrawData *pData)
{
    pData->BeginSection("global", "object");

    // Draw a marker to show we are being dragged
    //
    if (pData->DrawLayer("focus", true))
    {
        if (m_dragging)
        {
            pData->SetColor(RTK_RGB(128, 128, 255));
            pData->Ellipse(m_gx - m_drag_radius, m_gy - m_drag_radius,
                           m_gx + m_drag_radius, m_gy + m_drag_radius);
        }
    }

    pData->EndSection();

    // Get children to process message
    //
    for (int i = 0; i < m_child_count; i++)
        m_child[i]->OnUiUpdate(pData);
}


///////////////////////////////////////////////////////////////////////////
// Process GUI mouse messages
//
void CObject::OnUiMouse(RtkUiMouseData *pData)
{
    TRACE0("start");
    
    pData->BeginSection("global", "object");
    
    // Default process for any "move" modes
    // If we are a top-level object, 
    //
    if (pData->UseMouseMode("move") && m_parent == NULL)
    {
        // *** WARNING -- mouse should be made consistent with draw
        //
        double px = (double) pData->GetPoint().x / 1000;
        double py = (double) pData->GetPoint().y / 1000;
            
        if (pData->IsButtonDown())
        {
            double dx = px - m_gx;
            double dy = py - m_gy;
            double dist = sqrt(dx * dx + dy * dy);

            if (dist < m_drag_radius)
                m_dragging = true;
        }
        else if (pData->IsButtonUp())
            m_dragging = false;

        if (m_dragging)
        {
            // Set both local and global position, since they are the
            // same coord system
            //
            m_lx = px;
            m_ly = py;
            m_gx = px;
            m_gy = py;
        }
    }

    pData->EndSection();

    // Get children to process message
    //
    for (int i = 0; i < m_child_count; i++)
        m_child[i]->OnUiMouse(pData);
}

#endif
