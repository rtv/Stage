///////////////////////////////////////////////////////////////////////////
//
// File: gripperdevice.cc
// Author: brian gerkey
// Date: 09 Jul 2001
// Desc: Simulates a simple gripper
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/gripperdevice.cc,v $
//  $Author: gerkey $
//  $Revision: 1.23 $
//
///////////////////////////////////////////////////////////////////////////

//#define ENABLE_RTK_TRACE 1

#include <math.h>
#include <values.h>
#include "world.hh"
#include "gripperdevice.hh"
#include "raytrace.hh"

///////////////////////////////////////////////////////////////////////////
// Default constructor
//
CGripperDevice::CGripperDevice(CWorld *world, CEntity *parent )
  : CEntity(world, parent )
{
  m_data_len    = sizeof( player_gripper_data_t ); 
  m_command_len = sizeof( player_gripper_cmd_t ); 
  m_config_len  = 0;
  m_reply_len  = 0;

  // Set default shape and geometry
  this->shape = ShapeRect;
  this->size_x = 0.08;
  this->size_y = 0.30;
  
  // these are used both for locating break beams and for visualization
  this->m_paddle_width = this->size_x;
  this->m_paddle_height = this->size_y / 5.0;

  m_player.code = PLAYER_GRIPPER_CODE;
  this->stage_type = GripperType;
  this->color = ::LookupColor(GRIPPER_COLOR);

  m_interval = 0.1; 

  // we interact with pucks and obstacles
  puck_return = true; 
  obstacle_return = true;

  this->mass = 20; // same mass as a robot

  // some nice colors for beams
  clear_beam_color = ::LookupColor("light blue");
  broken_beam_color = ::LookupColor("red");
  
  // these are never changed (for now)
  m_paddles_moving = false;
  m_gripper_error = false;
  m_lift_up = false;
  m_lift_down = true;
  m_lift_moving = false;
  m_lift_error = false;
  
  // these are initial values
  m_paddles_open = true;
  m_paddles_closed = false;

  m_inner_break_beam = false;
  m_outer_break_beam = false;

  m_puck_count = 0;
}


///////////////////////////////////////////////////////////////////////////
// Load the entity from the world file
bool CGripperDevice::Load(CWorldFile *worldfile, int section)
{
  if (!CEntity::Load(worldfile, section))
    return false;

  if(!strcmp(worldfile->ReadString(section, "consume", "false"),"true"))
  {
    m_gripper_consume = true;
    m_puck_capacity = MAXGRIPPERCAPACITY;
  } 
  else
  {
    // default to the more common gripper
    m_gripper_consume = false;
    m_puck_capacity = 1;
  }
  return true;
}


///////////////////////////////////////////////////////////////////////////
// Update the device data
//
void CGripperDevice::Update( double sim_time )
{
  ASSERT(m_world != NULL);

  if(!Subscribed())
    return;
  
  
  // if its time to recalculate gripper state
  //
  if( sim_time - m_last_update <= m_interval )
    return;
  
  m_last_update = sim_time;
  
  // Get the command string
  //
  player_gripper_cmd_t cmd;
  int len = GetCommand(&cmd, sizeof(cmd));
  if (len > 0 && len != sizeof(cmd))
  {
    PRINT_MSG1("command buffer has incorrect length: %d -- ignored",len);
    return;
  }
  
  // Parse the command string (if there is one)
  //
  // this gripper only understands a subset of the P2 gripper commands.
  // also we ignore the cmd.arg byte, which can set actuation times
  // on the real gripper
  //
  if (len > 0)
  {
    switch(cmd.cmd)
    {
      case GRIPopen:
        if(!m_paddles_open)
        {
          m_paddles_open = true;
          m_paddles_closed = false;
          if(!m_gripper_consume)
            DropObject();
        }
        break;
      case GRIPclose:
        if(m_paddles_open)
        {
          PickupObject();
          m_paddles_open = false;
          m_paddles_closed = true;
        }
        break;
      case GRIPstop:
      case LIFTup:
      case LIFTdown:
      case LIFTstop:
      case GRIPstore:
      case GRIPdeploy:
      case GRIPhalt:
      case GRIPpress:
      case LIFTcarry:
        /*
        PRINT_MSG1("CGripperDevice::Update(): unimplemented gripper "
                   "command: %d\n", cmd.cmd);
         */
        break;
      default:
        /*
        PRINT_MSG1("CGripperDevice::Update(): unknown gripper "
                   "command: %d\n", cmd.cmd);
                   */
        break;
    }
  }

  m_outer_break_beam = BreakBeam(0) ? true : false;
  m_inner_break_beam = BreakBeam(1) ? true : false;

  // Construct the return data buffer
  //
  player_gripper_data_t data;
  MakeData(&data, sizeof(data));

  // Pass back the data
  //
  PutData(&data, sizeof(data));
}

// returns pointer to the puck in the beam, or NULL if none
CEntity* CGripperDevice::BreakBeam( int beam )
{
  double px, py, pth;
  GetGlobalPose( px, py, pth );

  double xoffset, yoffset;
  
  assert( beam < 2 );
  
  double costh = cos( pth );
  double sinth = sin( pth );

  // if we have a normal (non-consuming) gripper and the paddles are closed,
  // but we don't have a puck, then the beams are both clear. if we
  // have at least one puck, then both beams are broken
  if(!m_gripper_consume)
  {
    if(m_paddles_closed && !m_puck_count)
      return(NULL);
    else if(m_puck_count)
      return(m_pucks[0]);
  }

  double xdist, ydist;
  ydist = (this->size_y-2*this->m_paddle_height)/2.0;

  /*
  double xdist = 0;
  double ydist = this->m_paddle_height;
  */

  switch( beam )
  {
    case 0: 
      xdist = this->size_x/2.0 + this->m_paddle_width;
      break;
    case 1: 
      xdist = this->size_x/2.0 + this->m_paddle_width/2.0; 
      break;
    default:
      printf("unknown gripper break beam number %d\n", beam);
      return(NULL);
  }
  
  // transform to global coords
  xoffset = xdist * costh - ydist * sinth;
  yoffset = xdist * sinth + ydist * costh;

  double beamlength = this->size_y-2*this->m_paddle_height;

  CEntity* ent = 0;

  /*
  CLineIterator lit(px+xoffset, py+yoffset, pth - M_PI/2.0, 
                    this->size_y * 0.66, m_world->ppm, m_world->matrix, 
                    PointToBearingRange ); 
   */
  CLineIterator lit(px+xoffset, py+yoffset, pth - M_PI/2.0, 
                    beamlength, m_world->ppm, m_world->matrix, 
                    PointToBearingRange); 
  
  while( (ent = lit.GetNextEntity()) )
  {
    // grippers only perceive pucks right now.
    if( ent->stage_type == PuckType )
      return ent;
  }

  return NULL;
}

    
// Package up the gripper's state in the right format
//
void CGripperDevice::MakeData(player_gripper_data_t* data, size_t len)
{
  // check length first
  if(len != sizeof(player_gripper_data_t))
  {
    PRINT_MSG("CGripperDevice::MakeData(): got wrong-length data to fill\n");
    return;
  }

  // break beams are now implemented
  data->beams = 0;
  data->beams |=  m_outer_break_beam ? 0x04 : 0x00;
  data->beams |=  m_inner_break_beam ? 0x08 : 0x00;
  
  //   both beams are broken when we're holding a puck
  //data->beams |= (m_puck_count&&!m_gripper_consume) ? 0x04 : 0x00;
  //data->beams |= (m_puck_count&&!m_gripper_consume) ? 0x08 : 0x00;

  // set the proper bits
  data->state = 0;
  data->state |= m_paddles_open ? 0x01 : 0x00;
  data->state |= m_paddles_closed ? 0x02 : 0x00;
  data->state |= m_paddles_moving ? 0x04 : 0x00;
  data->state |= m_gripper_error ? 0x08 : 0x00;
  data->state |= m_lift_up ? 0x10 : 0x00;
  data->state |= m_lift_down ? 0x20 : 0x00;
  data->state |= m_lift_moving ? 0x40 : 0x00;
  data->state |= m_lift_error ? 0x80 : 0x00;
}
    
// Drop an object from the gripper (if there is one)
//
void CGripperDevice::DropObject()
{
  // if we don't have any pucks, then there are no pucks to drop
  if(!m_puck_count)
    return;

  // find out where we are
  double px,py,pth;
  GetGlobalPose(px,py,pth);

  // drop the last one we picked up
  double x_offset = (this->size_x*2.0);
  m_puck_count--;
  m_pucks[m_puck_count]->m_parent_entity = (CEntity*)NULL;
  m_pucks[m_puck_count]->SetDirty(1);
  m_pucks[m_puck_count]->SetGlobalPose(px+x_offset*cos(pth),
                                       py+x_offset*sin(pth),
                                       pth);
  m_pucks[m_puck_count]->SetGlobalVel(0, 0, 0);

  //printf("dropped puck %d at (%f,%f,%f) with speed %f\n",
         //m_pucks[m_puck_count],
         //px,py,pth,m_pucks[m_puck_count]->GetSpeed());
  //if(!m_puck_count)
  //REMOVE  expGripper.have_puck = false;
}
    
// Try to pick up an object with the gripper
//
void CGripperDevice::PickupObject()
{
  if(m_puck_count >= m_puck_capacity)
    return;
  
  // get our position
  double px,py,pth;
  GetGlobalPose(px,py,pth);

  // find the closest puck
  CEntity* closest_puck=NULL;
  //double closest_dist = MAXDOUBLE;
  //double ox,oy,oth;
  //double this_dist;

  // first, check the inner break beam
  if((closest_puck = BreakBeam(1)))
  {
    // make sure that we're not trying to pick up
    // an already picked up puck
    int j;
    for(j=0;j<m_puck_count;j++)
    {
      if(m_pucks[j] == closest_puck )
        break;
    }
    if(j < m_puck_count)
      closest_puck = NULL;
  }
  // if nothing there, check the outer break beam
  if(!closest_puck && (closest_puck = BreakBeam(0)))
  {
    // make sure that we're not trying to pick up
    // an already picked up puck
    int j;
    for(j=0;j<m_puck_count;j++)
    {
      if(m_pucks[j] == closest_puck )
        break;
    }
    if(j < m_puck_count)
      closest_puck = NULL;
  }

  if(closest_puck)
  {
    // pickup the puck
    closest_puck->m_parent_entity = this;

    // if we're consuming the puck then move it behind the gripper
    if(m_gripper_consume)
      closest_puck->SetPose(-this->size_x,0,0);
    else
      closest_puck->SetPose(this->size_x/2.0+closest_puck->size_x/2.0,0,0);

    m_pucks[m_puck_count++]=closest_puck;

    closest_puck->SetDirty(1);
  }
}

#ifdef INCLUDE_RTK2

///////////////////////////////////////////////////////////////////////////
// Initialise the rtk gui
void CGripperDevice::RtkStartup()
{
  CEntity::RtkStartup();
  
  // Create a figure representing this object
  this->grip_fig = rtk_fig_create(m_world->canvas, NULL, 49);
  // Set the color
  rtk_fig_color_rgb32(this->grip_fig, this->color);

  // Create a figure representing the inner breakbeam
  this->inner_beam_fig = rtk_fig_create(m_world->canvas, NULL, 49);
  // Create a figure representing the outer breakbeam
  this->outer_beam_fig = rtk_fig_create(m_world->canvas, NULL, 49);
}

///////////////////////////////////////////////////////////////////////////
// Finalise the rtk gui
void CGripperDevice::RtkShutdown()
{
  // Clean up the figure we created
  rtk_fig_destroy(this->grip_fig);

  CEntity::RtkShutdown();
} 

///////////////////////////////////////////////////////////////////////////
// Process GUI update messages
//
void CGripperDevice::RtkUpdate()
{
  CEntity::RtkUpdate();
  
  // Get global pose
  double gx, gy, gth;
  GetGlobalPose(gx, gy, gth);

  rtk_fig_clear(this->grip_fig);
  rtk_fig_origin(this->grip_fig, gx, gy, gth );

  // Draw the gripper
  //rtk_fig_rectangle(this->grip_fig, 0, 0, 0, this->size_x, this->size_y, 
                    //false);
    
  // locate the paddles
  double x_offset;
  double y_offset;
  if(this->m_paddles_open)
  {
    x_offset = (this->size_x/2.0)+(this->m_paddle_width/2.0);
    y_offset = (this->size_y/2.0)-(this->m_paddle_height/2.0);
  }
  else
  {
    x_offset = (this->size_x/2.0)+(this->m_paddle_width/2.0);
    y_offset = (this->m_paddle_height/2.0);
  }

  // Draw the paddles
  /*
  rtk_fig_rectangle(this->grip_fig,
                    (x_offset*cos(gth))+(y_offset*-sin(gth)),
                    (x_offset*sin(gth))+(y_offset*cos(gth)),
                    0, 
                    this->m_paddle_width,
                    this->m_paddle_height,
                    false);
                    */
  rtk_fig_rectangle(this->grip_fig,
                    x_offset, y_offset, 0,
                    this->m_paddle_width,
                    this->m_paddle_height,
                    false);
  rtk_fig_rectangle(this->grip_fig,
                    x_offset, -y_offset, 0,
                    this->m_paddle_width,
                    this->m_paddle_height,
                    false);

  rtk_fig_clear(this->inner_beam_fig);
  rtk_fig_clear(this->outer_beam_fig);
  
  // if a client is subscribed to this device, then draw break beams
  if(Subscribed() > 0 && m_world->ShowDeviceData(this->stage_type) )
  {
    rtk_fig_origin(this->inner_beam_fig, gx, gy, gth );
    rtk_fig_origin(this->outer_beam_fig, gx, gy, gth );

    double xdist, ydist;
    ydist = (this->size_y-2*this->m_paddle_height)/2.0;
    
    if(this->m_inner_break_beam)
    {
      // beam broken: set to red
      rtk_fig_color_rgb32(this->inner_beam_fig, this->broken_beam_color);
    }
    else
    {
      // beam unbroken: set to pale blue
      rtk_fig_color_rgb32(this->inner_beam_fig, this->clear_beam_color);
    }
   
    // draw inner beam
    xdist = this->size_x/2.0 + this->m_paddle_width/2.0;
    rtk_fig_line(this->inner_beam_fig, xdist, ydist, xdist, -ydist);

    if(this->m_outer_break_beam)
    {
      // beam broken: set to red
      rtk_fig_color_rgb32(this->outer_beam_fig, this->broken_beam_color);
    }
    else
    {
      // beam unbroken: set to pale blue
      rtk_fig_color_rgb32(this->outer_beam_fig, this->clear_beam_color);
    }

    // draw outer beam
    xdist = this->size_x/2.0 + this->m_paddle_width;
    rtk_fig_line(this->outer_beam_fig, xdist, ydist, xdist, -ydist);
  }
}

#endif

