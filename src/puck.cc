///////////////////////////////////////////////////////////////////////////
//
// File: puck.cc
// Author: brian gerkey
// Date: 4 Dec 2000
// Desc: Simulates the pucks
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/puck.cc,v $
//  $Author: gerkey $
//  $Revision: 1.2 $
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
#include "puck.hh"


///////////////////////////////////////////////////////////////////////////
// Default constructor
//
CPuck::CPuck(CWorld *world, CEntity *parent)
        : CEntity(world, parent)
{
    //m_beacon_id = 0;
    m_index = -1;
    
    // Set the initial map pose
    //
    m_map_px = m_map_py = m_map_pth = 0;

    exp.objectType = puck_o;
    exp.width = 0.1;
    exp.height = 0.1;
    strcpy( exp.label, "Puck" );
}


///////////////////////////////////////////////////////////////////////////
// Load the object from an argument list
//
bool CPuck::Load(int argc, char **argv)
{
    if (!CEntity::Load(argc, argv))
        return false;

    for (int i = 0; i < argc;)
    {
        // Extract id
        //
        //if (strcmp(argv[i], "id") == 0 && i + 1 < argc)
        //{
            //strcpy(m_id, argv[i + 1]);
            //m_beacon_id = atoi(argv[i + 1]);
            //i += 2;
        //}
        //else
            i++;
    }

#ifdef INCLUDE_RTK
    m_mouse_radius = (m_parent_object == NULL ? 0.2 : 0.0);
    m_draggable = (m_parent_object == NULL);
#endif
        
    return true;
}


///////////////////////////////////////////////////////////////////////////
// Save the object
//
bool CPuck::Save(int &argc, char **argv)
{
    if (!CEntity::Save(argc, argv))
        return false;

    // Save id
    //
    //char id[32];
    //snprintf(id, sizeof(id), "%d", (int) m_beacon_id);
    //argv[argc++] = strdup("id");
    //argv[argc++] = strdup(id);
    
    return true;
}


///////////////////////////////////////////////////////////////////////////
// Startup routine
//
bool CPuck::Startup()
{
    assert(m_world != NULL);
    m_last_time = 0;
    m_com_vr = 0;
    m_index = m_world->AddPuck(this);
    return CEntity::Startup();
}


///////////////////////////////////////////////////////////////////////////
// Update the puck
//
void CPuck::Update()
{
    //RTK_TRACE0("updating laser beacon");
    ASSERT(m_world != NULL);

    // Undraw our old representation
    //
    //m_world->SetCell(m_map_px, m_map_py, layer_puck, 0);
    m_world->SetCircle(m_map_px, m_map_py, exp.width, layer_puck, 0);

    // move, if necessary
    Move();      
    
    // Update our global pose
    //
    GetGlobalPose(m_map_px, m_map_py, m_map_pth);

    
    // Draw our new representation
    //
    //m_world->SetCell(m_map_px, m_map_py, layer_puck, 2);
    m_world->SetCircle(m_map_px, m_map_py, exp.width, layer_puck, 2);
    
    // write back into world array
    m_world->SetPuck(m_index, m_map_px, m_map_py, m_map_pth);
}

///////////////////////////////////////////////////////////////////////////
// Set the puck's speed
// 
void CPuck::SetSpeed(double vr)
{
  m_com_vr = vr;
}

///////////////////////////////////////////////////////////////////////////
// Check to see if the given pose will yield a collision
//
bool CPuck::InCollision(double px, double py, double pth)
{
    double qx = px;
    double qy = py;
    double sx = exp.width;
    double sy = exp.height;

    if (m_world->GetRectangle(qx, qy, pth, sx, sy, layer_obstacle) > 0)
        return true;
    
    return false;
}

///////////////////////////////////////////////////////////////////////////
// Move the puck
// 
void CPuck::Move()
{
    double step_time = m_world->GetTime() - m_last_time;
    m_last_time += step_time;

    if(m_com_vr < 0.01)
      return;
    // Get the current puck pose
    //
    double px, py, pth;
    GetGlobalPose(px, py, pth);

    // Compute a new pose
    // This is a zero-th order approximation
    //
    double qx = px + m_com_vr * step_time * cos(pth);
    double qy = py + m_com_vr * step_time * sin(pth);
    //double qth = pth + m_com_vth * step_time;
    double qth = pth;

    // Check for collisions
    // and accept the new pose if ok
    //
    if(InCollision(qx, qy, qth))
    {
    }
    // did we hit another puck?
    //  Solve this one later...
    /*else if(InCollisionWithPuck(qx,qy,qth))
    {
        // move the robot 
        SetPose(qx, qy, qth);
        
        // move the closest puck
        // 
        // NOTE: maybe not a complete solution; won't handle simultaneous
        //       puck impacts
        MovePuck(qx,qy,qth);
    }*/
    // everything's ok
    else
    {
        SetGlobalPose(qx, qy, qth);
    }

    // compute a new velocity, based on "friction"
    SetSpeed(max(0,m_com_vr - 0.1*m_com_vr));
}

///////////////////////////////////////////////////////////////////////////
// Return radius of puck
//
double CPuck::GetDiameter()
{
  return exp.width;
}

#ifdef INCLUDE_RTK

///////////////////////////////////////////////////////////////////////////
// Process GUI update messages
//
void CLaserBeacon::OnUiUpdate(RtkUiDrawData *data)
{
    CEntity::OnUiUpdate(data);

    data->begin_section("global", "");
    
    if (data->draw_layer("laser_beacon", true))
    {
        double r = 0.10;
        double ox, oy, oth;
        GetGlobalPose(ox, oy, oth);
        double dx = 2 * r * cos(oth);
        double dy = 2 * r * sin(oth);
        data->set_color(RTK_RGB(255, 0, 0));
        data->ellipse(ox - r, oy - r, ox + r, oy + r);
        data->line(ox - dx, oy - dy, ox + dx, oy + dy);
    }

    data->end_section();
}


///////////////////////////////////////////////////////////////////////////
// Process GUI mouse messages
//
void CLaserBeacon::OnUiMouse(RtkUiMouseData *data)
{
    CEntity::OnUiMouse(data);
}

#endif





