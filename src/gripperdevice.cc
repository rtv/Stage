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
//  $Revision: 1.2 $
//
///////////////////////////////////////////////////////////////////////////

//#define ENABLE_RTK_TRACE 1

#include <math.h>
#include <values.h>
#include "world.hh"
#include "gripperdevice.hh"


///////////////////////////////////////////////////////////////////////////
// Default constructor
//
CGripperDevice::CGripperDevice(CWorld *world, 
                               CEntity *parent, 
                               CPlayerServer* server)
        : CPlayerDevice(world, parent, server,
                        GRIPPER_DATA_START,
                        GRIPPER_TOTAL_BUFFER_SIZE,
                        GRIPPER_DATA_BUFFER_SIZE,
                        GRIPPER_COMMAND_BUFFER_SIZE,
                        GRIPPER_CONFIG_BUFFER_SIZE)
{
    m_update_interval = 0.1;
    m_last_update = 0;

    // these are never changed (for now)
    m_paddles_moving = false;
    m_gripper_error = false;
    m_lift_up = false;
    m_lift_down = false;
    m_lift_moving = false;
    m_lift_error = false;
    m_paddles_closed = false;
    
    // these are initial values
    m_paddles_open = true;
    
    // gui export stuff
    exporting = true;
    exp.objectType = gripper_o;
    exp.width = .05;
    exp.height = .15;

    m_gripper_range = 2.0*exp.width;
    m_puck_count = 0;
    m_puck_capacity = 2;

    // gripper specific gui stuff
    expGripper.paddle_width = exp.width;
    expGripper.paddle_height = exp.height / 4.0;
    expGripper.paddles_open = m_paddles_open;
    expGripper.paddles_closed = m_paddles_closed;
    expGripper.lift_up = m_lift_up;
    expGripper.lift_down = m_lift_down;

    exp.data = (char*)&expGripper;
    strcpy( exp.label, "Gripper" );
}

///////////////////////////////////////////////////////////////////////////
// Load the object from an argument list
//
bool CGripperDevice::Load(int argc, char **argv)
{
    if (!CPlayerDevice::Load(argc, argv))
        return false;

    for (int i = 0; i < argc;)
    {
        //if (strcmp(argv[i], "transparent") == 0)
        //{
            //m_transparent = true;
            //i += 1;
        //}
        //else
            i++;
    }
    return true;
}


///////////////////////////////////////////////////////////////////////////
// Update the device data
//
void CGripperDevice::Update()
{
    //RTK_TRACE0("updating");

    // Update children
    //
    CPlayerDevice::Update();

    // Dont update anything if we are not subscribed
    //
    if (!IsSubscribed())
        return;
    //RTK_TRACE0("is subscribed");
    
    ASSERT(m_server != NULL);
    ASSERT(m_world != NULL);
    
    // if its time to recalculate gripper state
    //
    if( m_world->GetTime() - m_last_update <= m_update_interval )
        return;
    m_last_update = m_world->GetTime();

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
            DropObject();
          }
          break;
        case GRIPclose:
          if(m_paddles_open)
          {
            m_paddles_open = false;
            PickupObject();
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
          PRINT_MSG1("CGripperDevice::Update(): unimplemented gripper "
                      "command: %d\n", cmd.cmd);
        default:
          PRINT_MSG1("CGripperDevice::Update(): unknown gripper "
                      "command: %d\n", cmd.cmd);
      }

      // This basically assumes instantaneous changes
      //
      expGripper.paddles_open = m_paddles_open;
    }

    // Construct the return data buffer
    //
    player_gripper_data_t data;
    MakeData(&data, sizeof(data));

    // Pass back the data
    //
    PutData(&data, sizeof(data));
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

  // break beams are not implemented
  data->beams = 0;

  // set the proper bits
  data->state = 0;
  data->state &= m_paddles_open ? 0x01 : 0x00;
  data->state &= m_paddles_closed ? 0x02 : 0x00;
  data->state &= m_paddles_moving ? 0x04 : 0x00;
  data->state &= m_gripper_error ? 0x08 : 0x00;
  data->state &= m_lift_up ? 0x10 : 0x00;
  data->state &= m_lift_down ? 0x20 : 0x00;
  data->state &= m_lift_moving ? 0x40 : 0x00;
  data->state &= m_lift_error ? 0x80 : 0x00;
}
    
// Drop an object from the gripper (if there is one)
//
void CGripperDevice::DropObject()
{
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
  double closest_dist = MAXDOUBLE;
  double ox,oy,oth;
  CEntity* this_puck=NULL;
  double this_dist;
  for(int i=0;(this_puck=m_world->GetPuck(i));i++)
  {
    // first make sure that we're not trying to pick up
    // an already picked up puck
    int j;
    for(j=0;j<m_puck_count;j++)
    {
      if(m_pucks[j] == this_puck)
        break;
    }
    if(j < m_puck_count)
      continue;
    this_puck->GetGlobalPose(ox,oy,oth);
    this_dist = sqrt((px-ox)*(px-ox)+(py-oy)*(py-oy));
    if(this_dist < closest_dist)
    {
      closest_dist = this_dist;
      closest_puck = this_puck;
    }
  }

  if(closest_puck && closest_dist<m_gripper_range)
  {
    printf("picking up puck %d\n", closest_puck);
    closest_puck->m_parent_object = this;
    closest_puck->SetPose(exp.width/2.0,0,0);
    m_pucks[m_puck_count++]=closest_puck;
  }
  else
  {
    puts("no close pucks");
  }
}

#ifdef INCLUDE_RTK

///////////////////////////////////////////////////////////////////////////
// Process GUI update messages
//
void CGripperDevice::OnUiUpdate(RtkUiDrawData *data)
{
    CEntity::OnUiUpdate(data);
    data->begin_section("global", "");
    
    if (data->draw_layer("", true))
    {
        double ox, oy, oth;
        GetGlobalPose(ox, oy, oth);
        data->set_color(RTK_RGB(0, 255, 0));
        // Draw the gripper
        data->ex_rectangle(ox, oy, oth, exp.width,exp.height);
        // Draw the paddles
        if(expGripper.paddles_open)
        {
          double x_offset = (exp.width/2.0)+(expGripper.paddle_width/2.0);
          double y_offset = (exp.height/2.0)-(expGripper.paddle_height/2.0);
          data->ex_rectangle(
              ox+(x_offset*cos(oth))+(y_offset*-sin(oth)),
              oy+(x_offset*sin(oth))+(y_offset*cos(oth)),
              oth, 
              expGripper.paddle_width,
              expGripper.paddle_height);

          y_offset = -(exp.height/2.0)+(expGripper.paddle_height/2.0);
          data->ex_rectangle(
              ox+(x_offset*cos(oth))+(y_offset*-sin(oth)),
              oy+(x_offset*sin(oth))+(y_offset*cos(oth)),
              oth, 
              expGripper.paddle_width,
              expGripper.paddle_height);
        }
        else
        {
          double x_offset = (exp.width/2.0)+(expGripper.paddle_width/2.0);
          double y_offset = (expGripper.paddle_height/2.0);
          data->ex_rectangle(
              ox+(x_offset*cos(oth))+(y_offset*-sin(oth)),
              oy+(x_offset*sin(oth))+(y_offset*cos(oth)),
              oth, 
              expGripper.paddle_width,
              expGripper.paddle_height);

          y_offset = -(expGripper.paddle_height/2.0);
          data->ex_rectangle(
              ox+(x_offset*cos(oth))+(y_offset*-sin(oth)),
              oy+(x_offset*sin(oth))+(y_offset*cos(oth)),
              oth, 
              expGripper.paddle_width,
              expGripper.paddle_height);
        }
    }

    data->end_section();
}


///////////////////////////////////////////////////////////////////////////
// Process GUI mouse messages
//
void CGripperDevice::OnUiMouse(RtkUiMouseData *pData)
{
    CEntity::OnUiMouse(pData);
}


#endif

