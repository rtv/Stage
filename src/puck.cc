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
//  $Revision: 1.16 $
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

  channel_return = 0; // default to visible on ACTS channel 0

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
        if (strcmp(argv[i], "friction") == 0 && i + 1 < argc)
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
    
    // Save radius
    //
    char z[128];
    snprintf(z, sizeof(z), "%0.2f", (double) m_size_x/2.0);
    argv[argc++] = strdup("radius");
    argv[argc++] = strdup(z);

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
    ASSERT(m_world != NULL);

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
// 
void CPuck::Move()
{
    double step_time = m_world->GetTime() - m_last_time;
    m_last_time += step_time;

    // don't move if we've been picked up
    if(m_parent_object) return;
    
    
    // Get the current puck pose
    //
    double px, py, pth;
    GetGlobalPose(px, py, pth);
    
    CCircleIterator cit( px, py, m_size_x/2.0, 
			 m_world->ppm, m_world->matrix );
    
    CEntity* ent;

    while( (ent = cit.GetNextEntity()) ) 
      if( ent && ent != this )
	{
 	  if( ent->puck_return ) 
  	    {

	      double impact_velocity = ent->GetSpeed();
	      double impact_mass = ent->GetMass();
	      
	      // Compute range and bearing FROM impacted ent
	      //
	      double ox,oy,oth;
	      ent->GetGlobalPose(ox,oy,oth);
	      
	      pth = atan2( py-oy, px-ox );
	      
	      if(impact_velocity)
		{
		  // we only have really big things(robots) and really small things
		  // (pucks).  
		  if(impact_mass > m_mass)
		    SetSpeed(2*fabs(impact_velocity));
		  else
		    SetSpeed(fabs(impact_velocity));
		  //printf("setting speed to: %f\n", 2*impact_velocity);
		}
	    }
  	  else if( ent->obstacle_return )
  	    {
  	      m_com_vr = 0; // a collision! stop moving
  	      return; // don;t move at all
  	    }
	}
    
    // lower bound on velocity
    if(m_com_vr < 0.01 ) m_com_vr = 0;
        
    if( m_com_vr != 0 ) // only move if we have some speed  
      {
	double qx = px + m_com_vr * step_time * cos(pth);
	double qy = py + m_com_vr * step_time * sin(pth);
	double qth = pth;
	
	SetGlobalPose( qx, qy, qth );
      }    
    
    // compute a new velocity, based on "friction"
    m_com_vr -= m_com_vr * m_friction;
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





