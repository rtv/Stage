///////////////////////////////////////////////////////////////////////////
//
// File: puck.cc
// Author: brian gerkey
// Date: 4 Dec 2000
// Desc: Simulates the pucks
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/puck.cc,v $
//  $Author: vaughan $
//  $Revision: 1.10 $
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
  m_stage_type = PuckType;

  m_channel = 0; // default to visible on ACTS channel 0

  m_size_x = 0.1;
  m_size_y = 0.1;

    m_friction = 0.05;
    // assume puck is 200g
    m_mass = 0.2;
    
    // Set the initial map pose
    //
    m_map_px = m_map_py = m_map_pth = 0;

    exp.objectType = puck_o;
    exp.width = m_size_x;
    exp.height = m_size_y;
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
        else if (strcmp(argv[i], "friction") == 0 && i + 1 < argc)
        {
            m_friction = atof(argv[i + 1]);
            i += 2;
        }
        else if (strcmp(argv[i], "radius") == 0 && i + 1 < argc)
        {
            exp.width = exp.height = 2*atof(argv[i + 1]);
            i += 2;

	    m_size_x = m_size_y = exp.width;
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
void CPuck::Update( double sim_time )
{
    static bool undrawn = false;

    if( Subscribed() < 1)
      return;
    
    ASSERT(m_world != NULL);
  
    // if its time to recalculate state
    //
    if( m_world->GetTime() - m_last_update <= m_interval )
      return;
    
    m_last_update = m_world->GetTime();
       
    // if we've been picked up by a gripper, then undraw once
    // and don't do anything else
    //
    if(m_parent_object)
    {
      if(!undrawn)
      {
        // Undraw our old representation
        //
        m_world->SetCircle(m_map_px, m_map_py, 
                           exp.width / 2.0, layer_puck, 0);
        m_world->SetCircle(m_map_px, m_map_py, 
                           exp.width / 2.0, layer_vision, 0);
        
        // should be able to set this flag and avoid constant undraws,
        // but it doesn't seem to work for some reason; pucks get left in 
        // the both the vision and puck layers...
        //undrawn = true;
      }

      return;
    }

    undrawn = false;

    Move();      

    // Undraw our old representation
    //
    m_world->SetCircle(m_map_px, m_map_py, exp.width / 2.0, layer_puck, 0);
    m_world->SetCircle(m_map_px, m_map_py, exp.width / 2.0, layer_vision, 0);

    
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
// Check to see if the given pose will yield a collision with an obstacle
//
// To save time, we'll do bounding box detection
bool CPuck::InCollision(double px, double py, double pth)
{
    if (m_world->GetRectangle(px, py, pth, .8*exp.width, .8*exp.width, 
                              layer_obstacle) > 0)
        return true;
    
    return false;
}

///////////////////////////////////////////////////////////////////////////
// Check to see if the given pose will yield a collision with a movable object
//
// we'll iterate through all the objects and assume that they're circular.
//   returns a pointer to the first colliding object in the list
CEntity* CPuck::InCollisionWithMovableObject(double px, double py, double pth)
{
  for(int i =0; i < m_world->GetObjectCount(); i++)
  {
    CEntity* object = m_world->GetObject(i);
    
    // ignore ourselves
    if(object == this)
      continue;

    // first match robots
    if(object->m_stage_type != ( RoundRobotType || RectRobotType ) )
      continue;

    // get the object's position
    double qx, qy, qth;
    object->GetGlobalPose(qx,qy,qth);
    
    // find distance to its center
    double dist = sqrt((px-qx)*(px-qx)+(py-qy)*(py-qy));

    // are we colliding?
    if(dist < ((object->m_size_x/2.0) + (m_size_x/2.0)))
    {
      return(object);
    }
  }
  for(int i =0; i < m_world->GetObjectCount(); i++)
  {
    CEntity* object = m_world->GetObject(i);
    
    // ignore ourselves
    if(object == this)
      continue;

    // now match pucks
    if(object->m_stage_type != PuckType )
      continue;

    // get the object's position
    double qx, qy, qth;
    object->GetGlobalPose(qx,qy,qth);
    
    // find distance to its center
    double dist = sqrt((px-qx)*(px-qx)+(py-qy)*(py-qy));

    // are we colliding?
    if(dist < ((object->m_size_x/2.0) + (m_size_x/2.0)))
    {
      return(object);
    }
  }
  return(NULL);
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

    // First, check to see if we hit another 
    // movable object (e.g., robot, puck)?
    //
    // if so, transfer momentum 
    if(CEntity* object = InCollisionWithMovableObject(qx,qy,qth))
    {
      double impact_velocity = object->GetSpeed();
      double impact_mass = object->GetMass();
      
      // Compute range and bearing FROM impacted object
      //
      double ox,oy,oth;
      object->GetGlobalPose(ox,oy,oth);
      double dx = qx - ox;
      double dy = qy - oy;
      double b = NORMALIZE(atan2(dy, dx));
      
      // Determine the direction we should move
      // It's the robot's orientation minus the bearing from the robot
      //   to the puck (loosely models circular objects colliding).
      double new_th = b;
      
      double new_x = qx;
      double new_y = qy;
      SetGlobalPose(new_x,new_y,new_th);

      if(impact_velocity)
      {
        // we only have really big things (robots) and really small things
        // (pucks).  
        if(impact_mass > m_mass)
          SetSpeed(2*fabs(impact_velocity));
        else
          SetSpeed(fabs(impact_velocity));
        //printf("setting speed to: %f\n", 2*impact_velocity);
      }
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
    SetSpeed(max(0,m_com_vr - m_friction*m_com_vr));
}

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





