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

#include <math.h>
#include "object.hh"

// *** HACK -- move this to a header
// Create an object given a type
//
CObject* CreateObject(const char *type, CWorld *world, CObject *parent);



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
    // Create child objects
    //
    CreateObjects();
    
    // Start each of the children we have just created
    //
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


///////////////////////////////////////////////////////////////////////////
// Create the objects by reading them from a file
//
bool CObject::CreateObjects()
{
    // *** HACK
    char *filename = "cave.cfg";

    // Open the configuration file
    //
    RtkCfgFile cfg_file;
    if (!cfg_file.Open(filename))
    {
        ERROR("Could not open config file");
        return false;
    }

    cfg_file.BeginSection(m_id);

    // Read in our own pose
    //   
    double px = cfg_file.ReadDouble("px", 0, "");
    double py = cfg_file.ReadDouble("py", 0, "");
    double pth = cfg_file.ReadDouble("pth", 0, "");
    SetPose(px, py, pth);

    cfg_file.EndSection();

    // Read in the objects from the configuration file
    //
    for (int i = 0; i < ARRAYSIZE(m_child); i++)
    {
        RtkString key; RtkFormat1(key, "child[%d]", (int) i);

        // Get the id of the i'th child
        //
        RtkString id = cfg_file.ReadString(m_id, CSTR(key), "", "");
        if (id == "")
            continue;

        // Get its type
        //
        RtkString type = cfg_file.ReadString(CSTR(id), "type", "", "");
        
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

    cfg_file.Close();
    
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
    if (pData->UseMouseMode("move") && m_parent == (CObject*) m_world)
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
