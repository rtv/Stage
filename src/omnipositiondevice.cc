///////////////////////////////////////////////////////////////////////////
//
// File: omnipositiondevice.cc
// Author: Andrew Howard
// Date: 19 Oct 2001
// Desc: Simulates an omni robot
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/omnipositiondevice.cc,v $
//  $Author: vaughan $
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

#include <math.h>
#include "world.hh"
#include "raytrace.hh"
#include "omnipositiondevice.hh"

#define TWOPI 2 * M_PI


///////////////////////////////////////////////////////////////////////////
// Constructor
COmniPositionDevice::COmniPositionDevice(CWorld *world, CEntity *parent)
  : CEntity( world, parent )
{    
    // set the Player IO sizes correctly for this type of Entity
    m_data_len = sizeof( player_position_data_t );
    m_command_len = sizeof( player_position_cmd_t );
    m_config_len = 0;
  
    m_player_type = PLAYER_POSITION_CODE; // from player's messages.h
  
    // set up our sensor response
    this->laser_return = LaserTransparent;
    this->sonar_return = true;
    this->obstacle_return = true;
    this->puck_return = true;

    m_com_vr = m_com_vth = 0;
    m_map_px = m_map_py = m_map_pth = 0;

    this->radius = 0.15;
    this->m_size_x = this->radius * 2;
    this->m_size_y = this->radius * 2;

    this->com_vx = this->com_vy = this->com_va = 0;
    this->odo_px = this->odo_py = this->odo_pa = 0;

    // update this device VERY frequently
    m_interval = 0.01; 
  
    // assume robot is 20kg
    m_mass = 20.0;
  
#ifdef INCLUDE_RTK
    m_mouse_radius = 0.400;
    m_draggable = true;
#endif
}

///////////////////////////////////////////////////////////////////////////
// Load the object from an argument list
bool COmniPositionDevice::Load(int argc, char **argv)
{
    if (!CEntity::Load(argc, argv))
        return false;

    /*
    for (int i = 0; i < argc;)
    {
        // Extract shape
        //
        if (strcmp(argv[i], "shape") == 0 && i + 1 < argc)
        {
          if(!strcmp(argv[i+1],"rectangle"))
            SetShape(rectangle);
          else if(!strcmp(argv[i+1],"circle"))
            SetShape(circle);
          else
            PRINT_MSG1("pioneermobilebase: unknown shape \"%s\"; "
                       "defaulting to rectangle", argv[i+1]);
          i += 2;
        }
        else
            i++;
    }
    */
    
    return true;
}


///////////////////////////////////////////////////////////////////////////
// Save the object
bool COmniPositionDevice::Save(int &argc, char **argv)
{
    if (!CEntity::Save(argc, argv))
        return false;

    /*
    argv[argc++] = strdup("shape");
    
    switch( m_shape )
      {
      case circle:
	argv[argc++] = strdup("circle");
	break;
      case rectangle:
	argv[argc++] = strdup("rectangle");
	break;
      default:
	printf( "Warning: wierd position device shape (%d)\n", m_shape );
	argv[argc++] = strdup("rectangle");
      }
    */
	
    return true;
}


///////////////////////////////////////////////////////////////////////////
// Update the device
void COmniPositionDevice::Update( double sim_time )
{
    ASSERT(m_world != NULL);
  
    // Dont do anything if its not time yet
    if (sim_time - m_last_update <  m_interval)
        return;
    m_last_update = sim_time;

    if (Subscribed() > 0)
    {  
        // Get the latest command
        if (GetCommand(&this->command, sizeof(this->command)) == sizeof(this->command))
            ParseCommandBuffer();
        
        // Move the robot
        Move();
        
        // Prepare data packet
        ComposeData();     
        PutData( &this->data, sizeof(this->data)  );
    }
    else  
    {
        // the device is not subscribed,
        // so reset odometry to default settings.
        this->odo_px = this->odo_py = this->odo_pa = 0;
    }

    // Get our new pose
    double px, py, pa;
    GetGlobalPose(px, py, pa);
    
    // if we've moved,
    // undraw then redraw ourself
    if((m_map_px != px) || (m_map_py != py) || (m_map_pth != pa))
    {
        Map(m_map_px, m_map_py, m_map_pth, false);
        m_map_px = px;
        m_map_py = py;
        m_map_pth = pa;
        Map(m_map_px, m_map_py, m_map_pth, true);
    }
}


///////////////////////////////////////////////////////////////////////////
// Compute the robots new pose
void COmniPositionDevice::Move()
{
    double step = m_world->m_sim_timestep;

    // Get the current robot pose
    //
    double px, py, pa;
    GetPose(px, py, pa);

    // Compute the new velocity
    // For now, just set to commanded velocity
    double vx = this->com_vx;
    double vy = this->com_vy;
    double va = this->com_va;
    
    // Compute a new pose
    // This is a zero-th order approximation
    double qx = px + step * vx * cos(pa) - step * vy * sin(pa);
    double qy = py + step * vx * sin(pa) + step * vy * cos(pa);
    double qa = pa + step * va;

    // Check for collisions
    // and accept the new pose if ok
    if (InCollision(qx, qy, qa))
    {
        this->stall = 1;
    }
    else
    {
        SetPose(qx, qy, qa);

        // if we moved, we mark ourselves dirty
        if( (px!=qx) || (py!=qy) || (pa!=qa) )
            MakeDirtyIfPixelChanged();

        // Compute the new odometric pose
        // Uses a zero-th order approximation
        this->odo_px += step * vx * cos(pa) - step * vy * sin(pa);
        this->odo_py += step * vx * sin(pa) + step * vy * cos(pa);
        this->odo_pa += pa + step * va;
        this->odo_pa = fmod(this->odo_pa + TWOPI, TWOPI);
        
        this->stall = 0;
    }
}


///////////////////////////////////////////////////////////////////////////
// Parse the command buffer to extract commands
void COmniPositionDevice::ParseCommandBuffer()
{
    double vx = (short) ntohs(this->command.speed);
    double vy = (short) ntohs(this->command.sidespeed);
    double va = (short) ntohs(this->command.turnrate);
    
    // Store commanded speed
    // Linear is in m/s
    // Angular is in radians/sec
    this->com_vx = vx / 1000;
    this->com_vy = vy / 1000;
    this->com_va = DTOR(va);
}


///////////////////////////////////////////////////////////////////////////
// Compose data to send back to client
void COmniPositionDevice::ComposeData()
{
    // Compute odometric pose
    // Convert mm and degrees (0 - 360)
    double px = this->odo_px * 1000.0;
    double py = this->odo_py * 1000.0;
    double pa = RTOD(this->odo_pa);

    // Construct the data packet
    // Basically just changes byte orders and some units
    this->data.xpos = htonl((int) px);
    this->data.ypos = htonl((int) py);
    this->data.theta = htons((unsigned short) pa);

    this->data.speed = htons((unsigned short) (this->com_vx * 1000.0));
    this->data.sidespeed = htons((unsigned short) (this->com_vy * 1000.0));
    this->data.turnrate = htons((short) RTOD(this->com_va));  
    this->data.compass = 0;
    this->data.stalls = this->stall;
}


///////////////////////////////////////////////////////////////////////////
// Check to see if the given pose will yield a collision
bool COmniPositionDevice::InCollision(double px, double py, double pa)
{
    CCircleIterator rit( px, py, this->radius,  
                         m_world->ppm, m_world->matrix );

    CEntity* ent;
	while ( (ent = rit.GetNextEntity()) )
        if( ent != this && ent->obstacle_return )
            return true;
	
	return false;
}


///////////////////////////////////////////////////////////////////////////
// Render the object in the world rep
void COmniPositionDevice::Map(double px, double py, double pa, bool render)
{
    if (!render)
        m_world->matrix->SetMode( mode_unset );      
    else
        m_world->matrix->SetMode( mode_set );
    
    m_world->SetCircle(px, py, this->radius, this );
}



#ifdef INCLUDE_RTK

///////////////////////////////////////////////////////////////////////////
// Process GUI update messages
//
void COmniPositionDevice::OnUiUpdate(RtkUiDrawData *data)
{
    CEntity::OnUiUpdate(data);
    
    data->begin_section("global", "");
    
    if (data->draw_layer("chassis", true))
        DrawChassis(data);
    
    data->end_section();
}


///////////////////////////////////////////////////////////////////////////
// Process GUI mouse messages
//
void COmniPositionDevice::OnUiMouse(RtkUiMouseData *data)
{
    CEntity::OnUiMouse(data);
}


///////////////////////////////////////////////////////////////////////////
// Draw the pioneer chassis
//
void COmniPositionDevice::DrawChassis(RtkUiDrawData *data)
{
    #define ROBOT_COLOR RTK_RGB(255, 0, 192)
    
    data->set_color(ROBOT_COLOR);

    // Get global pose
    //
    double px, py, pa;
    GetGlobalPose(px, py, pa);

    data->ellipse(px - m_size_x/2.0, py - m_size_x/2.0, 
                  px + m_size_x/2.0, py + m_size_x/2.0);
      
    // Draw the direction indicator
    //
    for (int i = 0; i < 3; i++)
    {
        double qx = m_size_x/2.0 * cos(DTOR(i * 45 - 45));
        double qy = m_size_x/2.0 * sin(DTOR(i * 45 - 45));
        double qth = 0;
        
        // This is ugly, but it works
        //
        if (i == 1)
          qx = qy = 0;

        LocalToGlobal(qx, qy, qth);

        if (i > 0)
            data->line(px, py, qx, qy);
        px = qx;
        py = qy;
    }
}

#endif
