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

#define ENABLE_TRACE 1

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
    m_child_count = 0;

    m_lx = m_ly = m_lth = 0;
    m_gx = m_gy = m_gth = 0;
    m_global_dirty = true;

#ifdef INCLUDE_RTK
    m_draggable = false;
    m_mouse_radius = 0;
    m_mouse_ready = false;
    m_dragging = false;
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
// Startup routine
//
bool CObject::Startup(RtkCfgFile *cfg)
{
    cfg->BeginSection(m_id);
   
    // Read in our own pose
    //   
    double px = cfg->ReadDouble("px", 0, "");
    double py = cfg->ReadDouble("py", 0, "");
    double pth = cfg->ReadDouble("pth", 0, "");
    SetPose(px, py, pth);

    cfg->EndSection();

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
// Recursive versions for standard functions
// These will call the function for all children, grand-children, etc,
// and will normally be called from the root object.
//
bool CObject::StartupChildren(RtkCfgFile *cfg)
{
    for (int i = 0; i < m_child_count; i++)
    {
        if (!m_child[i]->Startup(cfg))
            return false;
        if (!m_child[i]->StartupChildren(cfg))
            return false;
    }
    return true;
}
void CObject::ShutdownChildren()
{
    for (int i = 0; i < m_child_count; i++)
    {
        m_child[i]->Shutdown();
        m_child[i]->ShutdownChildren();
    }
}
void CObject::UpdateChildren()
{
    for (int i = 0; i < m_child_count; i++)
    {
        m_child[i]->Update();
        m_child[i]->UpdateChildren();
    }
}


///////////////////////////////////////////////////////////////////////////
// Create the objects by reading them from a file
// Creates objects recursively
//
bool CObject::CreateChildren(RtkCfgFile *cfg)
{ 
    // Read in the objects from the configuration file
    //
    for (int i = 0; i < ARRAYSIZE(m_child); i++)
    {
        RtkString key; RtkFormat1(key, "child[%d]", (int) i);

        // Get the id of the i'th child
        //
        RtkString id = cfg->ReadString(m_id, CSTR(key), "", "");
        if (id == "")
            continue;

        // Get its type
        //
        RtkString type = cfg->ReadString(CSTR(id), "type", "", "");
        
        // Create the object
        //
        MSG2("making object %s of type %s", CSTR(id), CSTR(type));
        CObject *object = ::CreateObject(CSTR(type), m_world, this);
        if (object == NULL)
            continue;

        // *** WARNING -- no overflow check!
        // Set the object's id
        //
        strcpy(object->m_id, CSTR(id));

        // Add into the object list
        //
        AddChild(object);
    }

    // Now get the children to create their children
    //
    for (int i = 0; i < m_child_count; i++)
    {
        if (!m_child[i]->CreateChildren(cfg))
            return false;
    }
    return true;
}


/////////////////////////////////////////////////////////////////////////
// Add a child object
//
void CObject::AddChild(CObject *child)
{
    ASSERT_INDEX(m_child_count, m_child);
    m_child[m_child_count++] = child;
}


/////////////////////////////////////////////////////////////////////////
// Get the first ancestor of the given run-time type
// Note: will return ourself if the type matches.
//
CObject* CObject::FindAncestor(const type_info &type)
{
    if (typeid(*this) == type)
        return this;
    else if (m_parent)
        return m_parent->FindAncestor(type);
    else
        return NULL;
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

    // Set our global pose
    //
    if (m_parent)
        m_parent->LocalToGlobal(px, py, pth);
    m_gx = px;
    m_gy = py;
    m_gth = pth;

    // Update all our children
    //
    for (int i = 0; i < m_child_count; i++)
        m_child[i]->SetPose(m_child[i]->m_lx, m_child[i]->m_ly, m_child[i]->m_lth);
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
    // Convert to local cs
    //
    if (m_parent)
        m_parent->GlobalToLocal(px, py, pth);

    // Set local pose
    //
    SetPose(px, py, pth);
}


///////////////////////////////////////////////////////////////////////////
// Get the objects pose in the global cs
//
void CObject::GetGlobalPose(double &px, double &py, double &pth)
{
    px = m_gx;
    py = m_gy;
    pth = m_gth;
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
    if (pData->DrawLayer("focus", true, 0))
    {
        if (m_mouse_ready)
            pData->SetColor(RTK_RGB(128, 128, 255));
        if (m_dragging)
            pData->SetColor(RTK_RGB(0, 0, 255));
        if (m_mouse_ready)
            pData->Ellipse(m_gx - m_mouse_radius, m_gy - m_mouse_radius,
                           m_gx + m_mouse_radius, m_gy + m_mouse_radius);
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
    pData->BeginSection("global", "object");
 
    // Default process for any "move" modes 
    //
    if (pData->UseMouseMode("move"))
    {
        // Get current pose
        //
        double px, py, pth;
        GetGlobalPose(px, py, pth);
    
        // Get the mouse position
        //
        double mx, my;
        pData->GetPoint(mx, my);
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
            if (pData->IsButtonDown())
            {
                // Drag on left
                //
                if (pData->WhichButton() == 1)
                    m_dragging = true;
                
                // Rotate on right
                //
                else if (pData->WhichButton() == 3)
                {
                    m_dragging = true;
                    mth += M_PI / 8;
                }
            }
            else if (pData->IsButtonUp())
                m_dragging = false;
        }

        // If we are dragging, set the pose
        //
        if (m_dragging)
            SetGlobalPose(mx, my, mth);
    }

    pData->EndSection();

    // Get children to process message
    //
    for (int i = 0; i < m_child_count; i++)
        m_child[i]->OnUiMouse(pData);
}

#endif
