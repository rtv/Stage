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
//  $Revision: 1.4 $
//
///////////////////////////////////////////////////////////////////////////

//#define ENABLE_RTK_TRACE 1

#include "world.hh"
#include "puck.hh"


///////////////////////////////////////////////////////////////////////////
// Default constructor
//
CPuck::CPuck(CWorld *world, CEntity *parent)
        : CEntity(world, parent)
{
    m_channel = 0;
    
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
        // Extract channel
        //
        if (strcmp(argv[i], "channel") == 0 && i + 1 < argc)
        {
            m_channel = atoi(argv[i + 1]) + 1;
            i += 2;
        }
        else
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
    return CEntity::Startup();
}


///////////////////////////////////////////////////////////////////////////
// Update the puck
//
void CPuck::Update()
{
    ASSERT(m_world != NULL);

    // Undraw our old representation
    //
    m_world->SetCircle(m_map_px, m_map_py, exp.width / 2.0, layer_puck, 0);
    m_world->SetCircle(m_map_px, m_map_py, exp.width / 2.0, layer_vision, 0);

    // move, if necessary (don't move if we've been picked up)
    if(!m_parent_object)
      Move();      
    
    // Grab our new global pose
    //
    GetGlobalPose(m_map_px, m_map_py, m_map_pth);

    
    // Draw our new representation
    //
    m_world->SetCircle(m_map_px, m_map_py, exp.width / 2.0, layer_puck, 2);
    m_world->SetCircle(m_map_px, m_map_py, exp.width / 2.0, layer_vision, 
                       m_channel);
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
// Check to see if the given pose will yield a collision with a movable object
//
bool CPuck::InCollisionWithMovableObject(double px, double py, double pth)
{
  if(m_world->GetRectangle(px, py, pth, exp.width, exp.width, layer_puck) > 0)
    return true;

  return(false);
}

///////////////////////////////////////////////////////////////////////////
// Move the puck
// 
void CPuck::Move()
{
    double step_time = m_world->GetTime() - m_last_time;
    m_last_time += step_time;

    // don't move if we've been picked up
    if(m_parent_object)
      return;
    
    // lower bound on velocity
    if(m_com_vr < 0.01)
      m_com_vr = 0;
    
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

    // did we hit another movable object (e.g., robot, puck)?
    if(InCollisionWithMovableObject(qx,qy,qth))
    {
      // transfer some momentum...
      //printf("Collision: %d\n",this);
    }
    // Did we hit a wall?
    // if so, don't move anymore (infinitely soft walls)
    //
    else if(InCollision(qx, qy, qth))
    {
      m_com_vr = 0;
    }
    // no collisions; make the move
    else
    {
        SetGlobalPose(qx, qy, qth);
    }

    // compute a new velocity, based on "friction"
    SetSpeed(max(0,m_com_vr - 0.1*m_com_vr));
}

///////////////////////////////////////////////////////////////////////////
// Search for which puck(s) we have collided with and move them
//
/*
void CPioneerMobileDevice::MovePuck(double px, double py, double pth)
{
    double min_range = MAXDOUBLE;
    double close_puck_bearing = 0;
    double close_puck_x = 0;
    double close_puck_y = 0;
    CPuck* close_puck = NULL;
    int close_puck_index = -1;
    
    //printf("robot pose: %f %f %f\n", px,py,RTOD(pth));
    // Search for the closest puck in the list
    // Is quicker than searching the bitmap!
    //
    for (int i = 0; true; i++)
    {
        // Get the position of the puck (global coords)
        //
        double qx, qy, qth;
        CPuck* tmp_puck;
        if (!(tmp_puck = m_world->GetPuck(i, &qx, &qy, &qth)))
            break;
        
        //printf("considering puck: %d\n", i);

        // Compute range and bearing to each puck
        //
        double dx = qx - px;
        double dy = qy - py;
        double r = sqrt(dx * dx + dy * dy);
        //printf("range: %f\n", r);
        double b = NORMALIZE(atan2(dy, dx) - pth);
        //double o = NORMALIZE(qth - pth);

        if(r < min_range)
        {
          min_range = r;
          close_puck_bearing = b;
          close_puck_x = qx;
          close_puck_y = qy;
          //close_puck_orientation = o;
          close_puck_index = i;
          close_puck = tmp_puck;
        }
    }

    if(close_puck_index == -1)
    {
      puts("couldn't find puck to move");
      return;
    }
    
    // determine the direction to move the puck.
    // it's the robot's orientation minus the bearing from the robot
    // to the puck (loosely models circular objects colliding)
    double puck_th = NORMALIZE(pth-close_puck_bearing);

    // set puck orientation (move it enough to get it away from the robot)
    close_puck->SetGlobalPose(close_puck_x+
                                2.0*(close_puck->GetDiameter())*cos(puck_th),
                              close_puck_y+
                                2.0*(close_puck->GetDiameter())*sin(puck_th),
                              puck_th);
    //close_puck->SetGlobalPose(close_puck_x,close_puck_y,puck_th);
    //printf("new puck pose: %f %f %f\n",
       //close_puck_x+2.0*(close_puck->GetDiameter())*cos(puck_th),
       //close_puck_y+2.0*(close_puck->GetDiameter())*sin(puck_th),
       //puck_th);

    // transfer some "momentum" (meters/second)
    // assume that the robot has 10 times the mass of the puck
    close_puck->SetSpeed(10*m_com_vr);
}
*/

#ifdef INCLUDE_RTK

///////////////////////////////////////////////////////////////////////////
// Process GUI update messages
//
void CPuck::OnUiUpdate(RtkUiDrawData *data)
{
    CEntity::OnUiUpdate(data);

    data->begin_section("global", "");
    
    if (data->draw_layer("", true))
    {
        double ox, oy, oth;
        GetGlobalPose(ox, oy, oth);
        double radius = GetDiameter() / 2.0;
        data->set_color(RTK_RGB(0, 0, 255));
        data->ellipse(ox - radius, oy - radius, 
                      ox + radius, oy + radius);
    }

    data->end_section();
}


///////////////////////////////////////////////////////////////////////////
// Process GUI mouse messages
//
void CPuck::OnUiMouse(RtkUiMouseData *data)
{
    CEntity::OnUiMouse(data);
}

#endif





