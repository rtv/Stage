//////////////////////////////////////////////////////////////////////////
//
// File: pioneermobiledevice.cc
// Author: Richard Vaughan, Andrew Howard
// Date: 5 Dec 2000
// Desc: Simulates the Pioneer robot base
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/models/descartesdevice.cc,v $
//  $Author: rtv $
//  $Revision: 1.1 $
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

//#define DEBUG
#undef DEBUG

#include "world.hh"
#include "descartesdevice.hh"
#include "raytrace.hh"

///////////////////////////////////////////////////////////////////////////
// Constructor
//
CDescartesDevice::CDescartesDevice(CWorld *world, CEntity *parent )
  : CPositionDevice( world, parent )
{    
  //puts( "DESCARTES" );

  // set the Player IO sizes correctly for this type of Entity
  m_data_len = sizeof( player_descartes_data_t );
  m_command_len = 0;//sizeof( player_descartes_cmd_t );
  m_config_len = 1;// not configurable
  m_reply_len = 1;

  m_player.code = PLAYER_DESCARTES_CODE; // from player's messages.h
  
  stage_type = DescartesType;
  
  this->color = ::LookupColor(DESCARTES_COLOR);

  // set up our sensor response
  laser_return = LaserTransparent;
  sonar_return = true;
  obstacle_return = true;
  puck_return = true;
  idar_return = IDARTransparent;

  // Set the robot dimensions
  size_x = 0.12;
  size_y = 0.12;
  origin_x = origin_y = 0;

  m_interval = 0.01; // update this device VERY frequently
  
  //robot is 0.5kg
  mass = 0.5;
  
  // Set the default shape
  shape = ShapeCircle;

  left_bumper = right_bumper = 0;

  goalspeed = goalheading = goaldistance = distance_travelled = 0;
}

///////////////////////////////////////////////////////////////////////////
// Startup routine
//
bool CDescartesDevice::Startup()
{
  if (!CPlayerEntity::Startup())
    return false;

  SetDriverName("hrl_descartes");
  return true;
}

///////////////////////////////////////////////////////////////////////////
// Update the position of the robot base
//
void CDescartesDevice::Update( double sim_time )
{
  //#ifdef DEBUG
  //puts( "STAGE: CDescartesDevice::Update()" );
  //#endif

  CEntity::Update( sim_time );


  ASSERT(m_world != NULL);
    
  // if its time to recalculate state
  //
  if( sim_time - m_last_update >  m_interval )
    {
      m_last_update = sim_time;

      if( Subscribed() > 0 )
	{
	  Servo(); // set speed and turnrate to achieve requested
	  // speed and heading
	  
	  Move();      // use CPositionDevice's movement model
	  
	  ComposeData();     // report the new state of things
	  PutData( &m_data, sizeof(m_data)  );     // generic device call
	}
      else  
	{
	  // the device is not subscribed,
	  // reset to default settings.
	  odo_px = odo_py = odo_pth = 0;
	}
    }
  
  
  double x, y, th;
  GetGlobalPose( x,y,th );
  ReMap( x, y, th );
}
///////////////////////////////////////////////////////////////////////////
// Update the position of the robot base
//
void CDescartesDevice::Sync( void )
{
  void *client;
  player_descartes_config_t cfg;
  
  // Get config
  int res = GetConfig( &client, &cfg, sizeof(cfg));
  
  switch( res )
    {   
    case -1: // error
      PRINT_ERR( "get config failed" );
      PutReply(client, PLAYER_MSGTYPE_RESP_NACK ); // not happy
      break;
      
    case 0:
      // nothing available - nothing to do
      break;
      
    case sizeof(cfg): // a good-sized config message!
      ParseConfig(&cfg);    // find out what to do    
      PutReply(client, PLAYER_MSGTYPE_RESP_ACK ); // say thankyou
      break;
      
    default:
      PRINT_ERR( "wierd result" );
      PutReply(client, PLAYER_MSGTYPE_RESP_NACK ); // not happy
      break;
    }
}  

// we override positiondevice's in collision method to detect which bumper
// was hit
CEntity* CDescartesDevice::TestCollision(double px, double py, double pth)
{
  //puts( "DESCARTES INCOLLISION" );
  double qx = px + this->origin_x * cos(pth) - this->origin_y * sin(pth);
  double qy = py + this->origin_y * sin(pth) + this->origin_y * cos(pth);
  double qth = pth;
  double sx = this->size_x;
  double sy = this->size_y;
  
  left_bumper = right_bumper = 0;
  
  switch( this->shape ) 
    {
    case ShapeRect:
      {
	CRectangleIterator rit( qx, qy, qth, sx, sy, m_world->ppm, m_world->matrix );
	
	CEntity* ent;
	while( (ent = rit.GetNextEntity()) )
	  {
	    if( ent != this && ent->obstacle_return )
	      {
		
		//find oout where we hit the object
		double hitx, hity;
		rit.GetPos( hitx, hity );
		
		// is this the left or right side?
		double hit_angle = atan2( hity-py, hitx-px ) - pth;
		
		while( hit_angle < 0 )
		  hit_angle += TWOPI;
		
		//intf( "HIT ANGLE: %.2f %s\n",
		//    hit_angle, hit_angle > M_PI ? "right" : "left" );
		
		left_bumper = hit_angle > M_PI ? 0 : 1;
		right_bumper = hit_angle > M_PI ? 1 : 0;
		
		return ent;
	      }
	  }
	return NULL;
      }
      case ShapeCircle:
	{
	  CCircleIterator rit( px, py, sx / 2, m_world->ppm, m_world->matrix );
	  
	  CEntity* ent;
	  while( (ent = rit.GetNextEntity()) )
	    {
	      if( ent != this && ent->obstacle_return )
		{
		  //printf( "hit ent %p (%d)\n",
		  //ent, ent->m_stage_type );
		  // find oout where we hit the object
		  double hitx, hity;
		  rit.GetPos( hitx, hity );
		  
		  // is this the left or right side?
		  double hit_angle = atan2( hity-py, hitx-px ) - pth;
		  
		  while( hit_angle < 0 )
		    hit_angle += TWOPI;
		  
		  //printf( "HIT ANGLE: %.2f %s\n",
		  //      hit_angle, hit_angle > M_PI ? "right" : "left" );
		  
		  left_bumper = hit_angle > M_PI ? 0 : 1;
		  right_bumper = hit_angle > M_PI ? 1 : 0;
		    
		  return ent;
		}
	    }
	  return NULL;
	}
    case ShapeNone:
      return NULL;
    }
  return NULL;
}

///////////////////////////////////////////////////////////////////////////
// Parse the command buffer to extract commands
//
void CDescartesDevice::ParseConfig( player_descartes_config_t* cfg )
{
  goalspeed = 0.001 * ntohs(cfg->speed); // convert mm to m
  goaldistance = 0.001 * ntohs(cfg->distance); // convert mm to m
  goalheading = DTOR(ntohs(cfg->heading)); // convert deg to rad
  
  distance_travelled = 0.0;
  
#ifdef DEBUG
  printf( "STAGE: Descartes command: speed:%.2f heading:%.2f(%.0f) distance:%.2f\n",
	  goalspeed, goalheading, RTOD(goalheading), goaldistance );
#endif

}

void CDescartesDevice::Servo()
{
  // we've gone this far since the last update
  double distance = hypot( odo_px - lastxpos, odo_py - lastypos );

  distance_travelled += distance;
  
  // remember this position
  lastxpos = odo_px;
  lastypos = odo_py;
  
  double heading_error = goalheading - odo_pth;

  double speed = 0, turnrate = 0;
  
  // handle the zero-crossing
  if( heading_error > M_PI ) 
    heading_error -= 2.0 * M_PI; 

  if( heading_error < -M_PI )
    heading_error += 2.0 * M_PI;

  // turn quickly on the spot to approx the right heading
  if( fabs(heading_error) > M_PI/10.0 )
    {
      turnrate = 0.5 * M_PI;
      //turnrate = 1.0 * M_PI;
       // turn right if the goal heading is that way
      if( heading_error < 0 ) turnrate *= -1.0;
    }
  else // proportional control for the last little correction
    {
      //turnrate = 1.0 * heading_error;
      turnrate = 1.0 * heading_error;
      // if we're very close to the right heading, move forward
      if( fabs( heading_error ) < M_PI/10.0 )
	speed = goalspeed;
    }

  // if we've gone far enough, stop.
if( distance_travelled >= goaldistance ) speed = 0;

//  #ifdef DEBUG
// printf( "CDescartesDevice::Servo() \n"
//	 "heading: %.2f (%.2f) "
//	 "speed: %.2f (%.2f) "
//	 "distance: %.2f (%.2f)\n"
//	 "herr = %.2f\n"
//	 "v = %.2f, w = %.2f\n",
//	 odo_pth, goalheading, 
//	 speed, goalspeed, 
//	 distance_travelled, goaldistance,
//	 heading_error, speed, turnrate);
//#endif
 
 // set the control variables
 com_vr = speed;
 com_vth = turnrate;
}

///////////////////////////////////////////////////////////////////////////
// Compose data to send back to client
//
void CDescartesDevice::ComposeData()
{
    // Compute odometric pose
    // Convert mm and degrees (0 - 360)
    //
    double px = odo_px * 1000.0;
    double py = odo_py * 1000.0;
    double pth = RTOD(fmod(odo_pth + TWOPI, TWOPI));

    // Construct the data packet
    // Basically just changes byte orders
    //
    m_data.xpos = htonl((int) px);
    m_data.ypos = htonl((int) py);
    m_data.theta = htons((unsigned short) pth);

    //printf( "\nodo_px: %.3f  odo_py: %.3f\n",
    //    odo_px, odo_py );
    //printf( "px: %d  py: %d\n",
    //   (int)px, (int)py );
    //printf( "m_data.xpos: %d  m_data.ypos: %d\n",
    //    m_data.xpos, m_data.ypos );

    //m_data.speed = htons((unsigned short) (com_vr * 1000.0));
    //m_data.distance_remaining

    //single byte - no swapping
    m_data.bumpers[0] = left_bumper;
    m_data.bumpers[1] = right_bumper;
}




#ifdef INCLUDE_RTK2

///////////////////////////////////////////////////////////////////////////
// Initialise the rtk gui
void CDescartesDevice::RtkStartup()
{
  CEntity::RtkStartup();

  // add a line indicating fowards
  double ox, oy, oth;
  this->GetPose( ox, oy, oth );
  
  double dx = this->size_x/2.0 * cos( oth );
  double dy = this->size_x/2.0 * sin( oth );
  
  rtk_fig_line( this->fig, 0,0, dx, dy );
}


///////////////////////////////////////////////////////////////////////////
// Finalise the rtk gui
void CDescartesDevice::RtkShutdown()
{
  CEntity::RtkShutdown();
} 


///////////////////////////////////////////////////////////////////////////
// Update the rtk gui
void CDescartesDevice::RtkUpdate()
{
  CEntity::RtkUpdate();   
}

#endif













