///////////////////////////////////////////////////////////////////////////
//
// File: omnipositiondevice.cc
// Author: Andrew Howard
// Date: 19 Oct 2001
// Desc: Simulates an omni robot
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/models/positiondevice.cc,v $
//  $Author: rtv $
//  $Revision: 1.11 $
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
#include "player.h"
#include "world.hh"
#include "raytrace.hh"
#include "positiondevice.hh"

#define THMAX(A,B) (A > B ? A : B)
#define THMIN(A,B) (A < B ? A : B)

///////////////////////////////////////////////////////////////////////////
// Constructor
CPositionDevice::CPositionDevice(LibraryItem* libit,CWorld *world, CEntity *parent)
  : CPlayerEntity( libit,world, parent )
{    
  // setup our Player interface for a POSITION device interface
  PositionInit();
  
  // set up our sensor response
  this->laser_return = LaserTransparent;
  this->sonar_return = true;
  this->obstacle_return = true;
  this->puck_return = true;

  // Set default shape and geometry
  this->shape = ShapeRect;
  this->size_x = 0.30;
  this->size_y = 0.30;

  this->com_vx = this->com_vy = this->com_va = 0;
  this->com_px = this->com_py = this->com_pa = 0;
  this->odo_px = this->odo_py = this->odo_pa = 0;

  // update this device VERY frequently
  m_interval = 0.01; 
  
  // assume robot is 20kg
  this->mass = 20.0;

  // default to velocity mode - the most common way to control a robot
  this->control_mode = VELOCITY_CONTROL_MODE;

  // OMNI_DRIVE_MODE = Omnidirectional drive - x y & th axes are
  // independent.  DIFF_DRIVE_MODE = Differential-drive - x & th axes
  // are coupled, y axis disabled
  //this->drive_mode = OMNI_DRIVE_MODE; // default can be changed in worldfile
  this->drive_mode = DIFF_DRIVE_MODE; // default can be changed in worldfile

  this->stall = false;
  this->motors_enabled = true;

  this->noise.frequency = STG_POSITION_FREQ; // out of 255
}

///////////////////////////////////////////////////////////////////////////
// Load the entity from the world file
bool CPositionDevice::Load(CWorldFile *worldfile, int section)
{
  if (!CPlayerEntity::Load(worldfile, section))
    return false;
  
  const char *rvalue;
  // initialize the string to our default mode
  rvalue = (this->drive_mode == OMNI_DRIVE_MODE) ? "omni" : "diff";
  
  rvalue = worldfile->ReadString(section, "drive", rvalue);
  
  if (strcmp(rvalue, "omni") == 0)
    this->drive_mode = OMNI_DRIVE_MODE;
  else if (strcmp(rvalue, "diff") == 0)
    this->drive_mode = DIFF_DRIVE_MODE;
  else
    {
      PRINT_WARN1( "Unknown drive mode (%s)in world file. "
		   "Using differential mode.", rvalue );
      this->drive_mode = DIFF_DRIVE_MODE;
    }
     
  return true;
}

///////////////////////////////////////////////////////////////////////////
// Startup routine
//
bool CPositionDevice::Startup()
{
  if (!CPlayerEntity::Startup())
    return false;

  return true;
}

///////////////////////////////////////////////////////////////////////////
// Update the device
void CPositionDevice::Update( double sim_time )
{  
  // Do some default processing
  CPlayerEntity::Update(sim_time);

  // Dont do anything if its not time yet
  if (sim_time - m_last_update <  m_interval)
    return;
  m_last_update = sim_time;

  if (Subscribed() > 0)
  {
    // Process configuration requests
    UpdateConfig();

    if( !PositionGetCommand( &this->com_px, &this->com_py, &this->com_pa, 
                             &this->com_vx, &this->com_vy, &this->com_va ) )
      PRINT_DEBUG( "** NO COMMAND AVAILABLE ** " );
    
    // update the robot's position
    if( this->motors_enabled )
    {
      if( this->control_mode == POSITION_CONTROL_MODE )
        // compute some velocities that'll move us to the goal position
        PositionControl();
	
      // and update our position based on the current velocities
      Move();
    }
  
    PositionPutData( this->odo_px, this->odo_py, this->odo_pa,
                     this->com_vx, this->com_vy,  this->com_va, 
                     this->stall );
  }
  else  
  {
    // TODO: this should be an option in the config file
    // the device is not subscribed,
    // so reset odometry to default settings.
    this->odo_px = this->odo_py = this->odo_pa = 0;

    // we go quiet when not subscribed
    this->noise.amplitude = 0; // out of 255
  }

  // Get our new pose
  double px, py, pa;
  GetGlobalPose(px, py, pa);

  // Remap ourselves if we have moved
  ReMap(px, py, pa);
}

  ///////////////////////////////////////////////////////////////////////////
  // Process configuration requests.
  void CPositionDevice::UpdateConfig()
  {
    int len;
    void *client;
    char buffer[PLAYER_MAX_REQREP_SIZE];

    while (true)
    {
      len = GetConfig(&client, buffer, sizeof(buffer));
      if (len <= 0)
        break;

      switch (buffer[0])
      {
        case PLAYER_POSITION_MOTOR_POWER_REQ:
          // motor state change request 
          // 1 = enable motors (default)
          // 0 = disable motors
          switch( buffer[1] )
          {
            case 0: this->motors_enabled = false;
              break;
            case 1: this->motors_enabled = true;
              break;
            default: 
              this->motors_enabled = false;
              PRINT_WARN1("Unrecognized motor state %d in"
                          " PLAYER_POSITION_MOTOR_POWER_REQ "
                          "(expecting 1(on) or 0(off). "
                          "Switching off motors for safety.", 
                          buffer[1] ); 
          }
          if( !this->motors_enabled ) SetGlobalVel( 0,0,0 );
	  
          PutReply(client, PLAYER_MSGTYPE_RESP_ACK);
          break;
	  
        case PLAYER_POSITION_RESET_ODOM_REQ:
          // reset position to 0,0,0
          this->odo_px = this->odo_py = this->odo_pa = 0.0;
          PutReply(client, PLAYER_MSGTYPE_RESP_ACK);
          break;
	  
        case PLAYER_POSITION_SET_ODOM_REQ:
          // set my odometry estimate to the values in the packet
          PositionSetOdomReqUnpack( (player_position_set_odom_req_t*)buffer,
                                    &this->odo_px, 
                                    &this->odo_py, 
                                    &this->odo_pa );
	     
          PutReply(client, PLAYER_MSGTYPE_RESP_ACK);
          break;
	  
        case PLAYER_POSITION_GET_GEOM_REQ:
        {
          // Return the robot geometry (rotation offsets and size)
          player_position_geom_t geom;	  
          PositionGeomPack( &geom, 
                            this->origin_x, this->origin_y, 0,
                            this->size_x, this->size_y );
	    
          PutReply(client, PLAYER_MSGTYPE_RESP_ACK, NULL, &geom, sizeof(geom));
        }
        break;

        case PLAYER_POSITION_POSITION_MODE_REQ:
          // switch between velocity and position control
          switch( buffer[1] )
          {
            case 0: this->control_mode = VELOCITY_CONTROL_MODE; 
              break;
            case 1: this->control_mode = POSITION_CONTROL_MODE; 
              break;
            default: 
              PRINT_WARN2("Unrecognized mode in"
                          " PLAYER_POSITION_POSITION_MODE_REQ (%d)."
                          "Sticking with current mode (%s)", 
                          buffer[0], 
                          this->control_mode ? 
                          "POSITION_CONTROL_MODE" : "VELOCITY_CONTROL_MODE" ); 
              break;
          }
          PutReply(client, PLAYER_MSGTYPE_RESP_ACK);
          break;

          // CONFIGS NOT IMPLEMENTED
	
        case PLAYER_POSITION_POSITION_PID_REQ:
          PRINT_WARN( "PLAYER_POSITION_POSITION_PID_REQ has no effect" );
          PutReply(client, PLAYER_MSGTYPE_RESP_ACK);
          break;

        case PLAYER_POSITION_SPEED_PID_REQ:
          PRINT_WARN( "PLAYER_POSITION_SPEED_PID_REQ has no effect" );
          PutReply(client, PLAYER_MSGTYPE_RESP_ACK);
          break;

        case PLAYER_POSITION_SPEED_PROF_REQ:
          PRINT_WARN( "PLAYER_POSITION_SPEED_PROF_REQ has no effect" );
          PutReply(client, PLAYER_MSGTYPE_RESP_ACK);
          break;

        case PLAYER_POSITION_VELOCITY_MODE_REQ:
          // velocity control mode:
          //   0 = direct wheel velocity control (default)
          //   1 = separate translational and rotational control
          PRINT_WARN( "PLAYER_POSITION_VELOCITY_MODE_REQ has no effect" );
          PutReply(client, PLAYER_MSGTYPE_RESP_ACK);
          break;

        default:
          PRINT_WARN1("got unknown config request \"%c\"\n", buffer[0]);
          PutReply(client, PLAYER_MSGTYPE_RESP_NACK);
          break;
	    }
    }
  }

///////////////////////////////////////////////////////////////////////////
// Compute the robots new pose
void CPositionDevice::Move()
{
  double step = m_world->m_sim_timestep;

  // Get the current robot pose
  //
  double px, py, pa;
  GetPose(px, py, pa);
  
  //fprintf( stderr, "%.2f %.2f %.2f   %.2f %.2f %.2f\n", 
  //   px -1, py -1, pa, odo_px, odo_py, odo_pa );
  
  // Compute the new velocity
  // For now, just set to commanded velocity
  double vx = this->com_vx;
  double vy = this->com_vy;
  double va = this->com_va;
  
  // Compute movement deltas
  // This is a zero-th order approximation
  double dx=0, dy=0, da=0;

  switch( this->drive_mode )
  {
    case OMNI_DRIVE_MODE: // omnidirectional - axes independent
      dx = step * vx * cos(pa) - step * vy * sin(pa);
      dy = step * vx * sin(pa) + step * vy * cos(pa);
      da = step * va;
      break;
      
    case DIFF_DRIVE_MODE: //differential steering - no vy allowed
      dx = step * vx * cos(pa);
      dy = step * vx * sin(pa);
      da = step * va;
      break;
      
    default:
      PRINT_WARN1( "unknown drive mode (%d)", this->drive_mode );
      break;
  }
  
  // compute a new pose
  double qx = px + dx;
  double qy = py + dy;
  double qa = pa + da;

  // Check for collisions
  // and accept the new pose if ok
  if (TestCollision(qx, qy, qa ) != NULL)
  {
    SetGlobalVel(0, 0, 0);
    this->stall = true;

    // we go quiet when stalled 
    this->noise.amplitude = 0; // out of 255
  }
  else
  {
    // set pose now takes care of marking us dirty
    SetPose(qx, qy, qa);
    SetGlobalVel(vx, vy, va);
        
    // Compute the new odometric pose
    // we currently have PERFECT odometry. yum!
    this->odo_px += 
      step * vx * cos(this->odo_pa) + step * vy * sin(this->odo_pa);
    this->odo_py += 
      step * vx * sin(this->odo_pa) + step * vy * cos(this->odo_pa);
    this->odo_pa += step * va;
    
    this->stall = false;

    // we're noisy while driving
    this->noise.amplitude = STG_POSITION_AMP; // out of 255
  }
}

///////////////////////////////////////////////////////////////////////////
// Generate sensible velocities to achieve the goal pose
void CPositionDevice::PositionControl()
{
  const double DISTANCE_THRESHOLD = 0.02;
  const double ANGLE_THRESHOLD = 0.1;

  // normalize odo_pa to +-PI
  this->odo_pa = NORMALIZE(this->odo_pa);

  // Get the pose error
  double error_x = this->com_px - this->odo_px;
  double error_y = this->com_py - this->odo_py;
  double error_a = this->com_pa - this->odo_pa;

  // handle the zero-crossing
  error_a = NORMALIZE(error_a);
  
  //printf( "Odo: %.2f %.2f %.2f\n", odo_px, odo_py, odo_pa );
  //printf( "Errors: %.2f %.2f %.2f\n", error_x, error_y, error_a );

  // set the translation speeds
  if( this->drive_mode )
  {
    // set speeds proportional to error
    this->com_vx = error_x;
    this->com_vy = error_y;
    this->com_va = error_a;
  }
  else // a little more complex with a diff steer robot - this
    // controller seems to work ok
  {
    // find the angle to the goal
    double error_a_goal = atan2( error_y, error_x ) - this->odo_pa;
    // find the distance from the goal
    double error_d_goal = hypot( error_y, error_x );

    // if the goal is behind us, we'll go backwards!
    if( error_a_goal > M_PI/2.0 || error_a_goal < -M_PI/2 )
    {
      //puts( "BACKWARDS" );
      error_d_goal = -error_d_goal; // negative speed
      error_a_goal = NORMALIZE( error_a_goal + M_PI );// invert heading
    }
    //else
    //puts( "FORWARDS" );
      
    // if we're at the goal, turn to face the right direction
    if( fabs(error_d_goal) < DISTANCE_THRESHOLD )
    {
      //puts( "turning to the right heading" );
      this->com_vx = error_d_goal;
      this->com_vy = 0.0;
      this->com_va = error_a;
    }
    else // if we're pointing towards the goal, move towards it 
      if( fabs(error_a_goal) < ANGLE_THRESHOLD )
  	  {
  	    //puts( "moving towards goal point" );
  	    this->com_vx = error_d_goal;
  	    this->com_vy = 0.0;
  	    this->com_va = error_a_goal; // keep turning towards the goal
  	  }
      else // turn towards the goal
      {
        //puts( "turning towards goal point" );
        this->com_va = error_a_goal;
        this->com_vx = 0.0;
        this->com_vy = 0.0;
      }
  }

  // threshold speeds
  this->com_vx = THMIN(this->com_vx,  0.5);
  this->com_vx = THMAX(this->com_vx, -0.5);
  this->com_vy = THMIN(this->com_vy,  0.5);
  this->com_vy = THMAX(this->com_vy, -0.5);
  this->com_va = THMIN(this->com_va,  1.0);
  this->com_va = THMAX(this->com_va, -1.0);
}

#ifdef INCLUDE_RTK2

///////////////////////////////////////////////////////////////////////////
// Initialise the rtk gui
void CPositionDevice::RtkStartup()
{
  CPlayerEntity::RtkStartup();
  
  // add a 'nose line' indicating forward to the entity's normal
  // rectangle or circle. draw from the center of rotation to the front.
  rtk_fig_line( this->fig, 
		this->origin_x, this->origin_y, 
		this->size_x/2.0, this->origin_y );
}


#endif


