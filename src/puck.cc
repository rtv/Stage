///////////////////////////////////////////////////////////////////////////
//
// File: puck.cc
// Author: brian gerkey
// Date: 4 Dec 2000
// Desc: Simulates the pucks
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/puck.cc,v $
//  $Author: ahoward $
//  $Revision: 1.21.2.1 $
//
///////////////////////////////////////////////////////////////////////////

//#define ENABLE_RTK_TRACE 1

#include "world.hh"
#include "puck.hh"
#include "raytrace.hh"

///////////////////////////////////////////////////////////////////////////
// Default constructor
//
CPuck::CPuck(CWorld *world, CEntity *parent)
        : CEntity(world, parent)
{
  m_stage_type = PuckType;
  m_color_desc = PUCK_COLOR;

  this->vision_return = true;
  this->vision_return_held = false;
  this->vision_return_notheld = true;

  this->shape = ShapeCircle;
  m_size_x = 0.1;
  m_size_y = 0.1;

  m_interval = 0.01; // update very fast!

  obstacle_return = false; // we don't stop robots
  puck_return = true; // yes! we interact with pucks!
  
  m_friction = 0.05;
  // assume puck is 200g
  m_mass = 0.2;
  
  dx = dy = 0;

  // Set the initial map pose
  //
  m_map_px = m_map_py = m_map_pth = 0;
  
  exp.objectType = puck_o;
  exp.width = m_size_x;
  exp.height = m_size_y;
  strcpy( exp.label, "Puck" );

  m_player_port = 0; // not a player device
  m_player_type = 0;
  m_player_index = 0;
}


///////////////////////////////////////////////////////////////////////////
// Load the entity from the worldfile 
bool CPuck::Load(CWorldFile *worldfile, int section)
{
  if (!CEntity::Load(worldfile, section))
    return false;

  // Puck friction parameter
  m_friction = worldfile->ReadFloat(0, "puck_friction", m_friction);
  m_friction = worldfile->ReadFloat(section, "friction", m_friction);

  // Get vision held/not held values
  // TODO

  return true;
}


///////////////////////////////////////////////////////////////////////////
// Startup routine
bool CPuck::Startup()
{
  assert(m_world != NULL);
    
  // REMOVE m_com_vr = 0;

  return CEntity::Startup();
}


///////////////////////////////////////////////////////////////////////////
// Update the puck
void CPuck::Update( double sim_time )
{
  ASSERT(m_world != NULL);

  // have we been picked up?  then make ourselves transparent so the vision
  // doesn't see us in the gripper. otherwise make ourselves visible.
  if(m_parent_object)
    this->vision_return = this->vision_return_held;
  else if(!m_parent_object)
    this->vision_return = this->vision_return_notheld;

  // if its time to recalculate state and we're on the ground
  //
  if( (sim_time - m_last_update > m_interval) && (!m_parent_object) )
  {	
    m_last_update = sim_time;
	
    Move();      
  }
    
  double x, y, th;
  GetGlobalPose( x,y,th );
    
  // if we've moved 
  if( (m_map_px != x) || (m_map_py != y) || (m_map_pth != th ) )
  {
    // Undraw our old representation
    m_world->matrix->SetMode( mode_unset );
    m_world->SetCircle( m_map_px, m_map_py, m_size_x/2.0, this );
	
    m_map_px = x; // update the render positions
    m_map_py = y;
    m_map_pth = th;
	
    // Draw our new representation
    m_world->matrix->SetMode( mode_set );
    m_world->SetCircle( m_map_px, m_map_py, m_size_x/2.0, this );
  }
}


///////////////////////////////////////////////////////////////////////////
// Move the puck
void CPuck::Move()
{
  double step_time = m_world->m_sim_timestep;

  // don't move if we've been picked up
  if(m_parent_object) return;
    
  // Get the current puck pose
  double px, py, pth;
  GetGlobalPose(px, py, pth);
    
  CCircleIterator cit( px, py, m_size_x/2.0, 
                       m_world->ppm, m_world->matrix );
  CEntity* ent;

  while( (ent = cit.GetNextEntity()) )
  {
    if( ent && ent != this )
    {
      if( ent->puck_return ) 
      {
        // Get the position of the thing we hit
        double ox,oy,oth;
	      ent->GetGlobalPose(ox,oy,oth);
        
        // Get the velocity of the thing we hit
        double wx, wy, wth;
        ent->GetGlobalVel(wx, wy, wth);
	      double impact_velocity = sqrt(wx * wx + wy * wy);
        
        // Get the mass of the thing we hit
        double impact_mass = ent->GetMass();
	      
	      // Compute bearing FROM impacted ent
        // Align ourselves so we are facing away from this point
	      pth = atan2( py-oy, px-ox );
	      
	      if(impact_velocity)
        {
          // we only have really big things(robots) and really small things
          // (pucks).  
          double vr = 0;
          if(impact_mass > m_mass)
            vr = 2*fabs(impact_velocity);
          else
            vr = fabs(impact_velocity);
          SetGlobalVel(vr * cos(pth), vr * sin(pth), 0);
          //printf("setting speed to: %f\n", 2*impact_velocity);
        }
	    }
  	  else if( ent->obstacle_return )
      {
        // If we have collided, stop moving
        SetGlobalVel(0, 0, 0);
        // REMOVE return; // don;t move at all
      }
    }
  }

  // Get our current vel
  double vx, vy, vth;
  GetGlobalVel(vx, vy, vth);
  double vr = sqrt(vx * vx + vy * vy);
  
  // lower bound on velocity
  if (vr < 0.01 )
    SetGlobalVel(0, 0, 0);
        
  if( vr != 0 ) // only move if we have some speed  
  {
    double qx = px + step_time * vx;
    double qy = py + step_time * vy;
    double qth = pth;
    SetGlobalPose( qx, qy, qth );

    // compute a new velocity, based on "friction"
    vr -= vr * m_friction;
    vx = vr * cos(pth);
    vy = vr * sin(pth);
    SetGlobalVel( vx, vy, vth);

    // if we moved, we mark ourselves dirty
    if( (px!=qx) || (py!=qy) || (pth!=qth) )
      MakeDirtyIfPixelChanged();
  }    
}

#ifdef INCLUDE_RTK

///////////////////////////////////////////////////////////////////////////
// Process GUI update messages
void CPuck::OnUiUpdate(RtkUiDrawData *data)
{
  CEntity::OnUiUpdate(data);

  data->begin_section("global", "");
    
  if (data->draw_layer("", true))
  {
    double ox, oy, oth;
    GetGlobalPose(ox, oy, oth);
    double radius = GetDiameter() / 2.0;
    data->set_color(RTK_RGB(m_color.red, m_color.green, m_color.blue));
    data->ellipse(ox - radius, oy - radius, ox + radius, oy + radius);
  }

  data->end_section();
}


///////////////////////////////////////////////////////////////////////////
// Process GUI mouse messages
void CPuck::OnUiMouse(RtkUiMouseData *data)
{
  CEntity::OnUiMouse(data);
}

#endif





