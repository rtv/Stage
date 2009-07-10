/////////////////////////////////
// File: stest.c
// Desc: Stage library test program
// Created: 2004.9.15
// Author: Richard Vaughan <vaughan@sfu.ca>
// CVS: $Id: stest.cc,v 1.3 2008-02-01 03:11:02 rtv Exp $
// License: GPL
/////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stage.hh"
using namespace Stg;

typedef struct
{
  ModelLaser* laser;
  ModelPosition* position;
  ModelRanger* ranger;
  ModelFiducial* fiducial;

  ModelFiducial::Fiducial* closest;  
  stg_radians_t closest_bearing;
  stg_meters_t closest_range;
  stg_radians_t closest_heading_error; 

} robot_t;


const double VSPEED = 0.3; // meters per second
const double WGAIN = 0.3; // turn speed gain
const double SAFE_DIST = 1.0; // meters
const double SAFE_ANGLE = 0.5; // radians


// forward declare
int RangerUpdate( ModelRanger* mod, robot_t* robot );
int FiducialUpdate( ModelFiducial* fid, robot_t* robot );

// Stage calls this when the model starts up
extern "C" int Init( Model* mod )
{
  robot_t* robot = new robot_t;
  robot->position = (ModelPosition*)mod;

  // subscribe to the ranger, which we use for navigating
  robot->ranger = (ModelRanger*)mod->GetModel( "ranger:0" );
  assert( robot->ranger );
  robot->ranger->Subscribe();
  
  // ask Stage to call into our ranger update function
  robot->ranger->AddUpdateCallback( (stg_model_callback_t)RangerUpdate, robot );
 
  robot->fiducial = (ModelFiducial*)mod->GetModel( "fiducial:0" ) ;
  assert( robot->fiducial );
  robot->fiducial->AddUpdateCallback( (stg_model_callback_t)FiducialUpdate, robot );
  robot->fiducial->Subscribe();

  // subscribe to the laser, though we don't use it for navigating
  //robot->laser = (ModelLaser*)mod->GetModel( "laser:0" );
  //assert( robot->laser );
  //robot->laser->Subscribe();

  return 0; //ok
}

int RangerUpdate( ModelRanger* rgr, robot_t* robot )
{  	
  // compute the vector sum of the sonar ranges	      
  double dx=0, dy=0;
  
  // use the front-facing sensors only
  for( unsigned int i=0; i < 8; i++ )
	 {
		ModelRanger::Sensor& s = rgr->sensors[i];
		dx += s.range * cos( s.pose.a );
		dy += s.range * sin( s.pose.a );
		
		//printf( "sensor %d angle= %.2f\n", s, rgr->sensors[s].pose.a );	 
	 }
  
  if( (dx == 0) || (dy == 0) )
	 return 0;
    
  double resultant_angle = atan2( dy, dx );
  double forward_speed = 0.0;
  double side_speed = 0.0;	   
  double turn_speed = WGAIN * resultant_angle;
  
  //printf( "resultant %.2f turn_speed %.2f\n", resultant_angle, turn_speed );
  
  // if the front is clear, drive forwards
  if( (rgr->sensors[3].range > SAFE_DIST) && // forwards
		(rgr->sensors[4].range > SAFE_DIST) &&
		(rgr->sensors[5].range > SAFE_DIST/2.0) && //
		(rgr->sensors[6].range > SAFE_DIST/4.0) && 
		(rgr->sensors[2].range > SAFE_DIST/2.0) && 
		(rgr->sensors[1].range > SAFE_DIST/4.0) && 
		(fabs( resultant_angle ) < SAFE_ANGLE) )
	 {
		forward_speed = VSPEED;

		// and steer to match the heading of the nearest robot
		if( robot->closest )
		  turn_speed = WGAIN * robot->closest_heading_error;
	 }
  
  robot->position->SetSpeed( forward_speed, side_speed, turn_speed );

  return 0;
}


int FiducialUpdate( ModelFiducial* fid, robot_t* robot )
{  	
  // find the closest teammate

  //puts( "fiducial update" );
    
  double dist = 1e6; // big
  
  robot->closest = NULL;
  
  FOR_EACH( it, fid->GetFiducials() )
	 {
		ModelFiducial::Fiducial* other = &(*it);
		
		if( other->range < dist )
		  {
			 dist = other->range;
			 robot->closest = other;
		  }				
	 }
  
  if( robot->closest ) // if we saw someone
	 {
		//printf( "model %s see closest %s\n", fid->Token(), robot->closest->mod->Token() );

		robot->closest_bearing = robot->closest->bearing;
		robot->closest_range = robot->closest->range;
		robot->closest_heading_error = robot->closest->geom.a;
	 }
    
//   if( (dx == 0) || (dy == 0) )
// 	 return 0;
    
//   double resultant_angle = atan2( dy, dx );
//   double forward_speed = 0.0;
//   double side_speed = 0.0;	   
//   double turn_speed = WGAIN * resultant_angle;
  
//   //printf( "resultant %.2f turn_speed %.2f\n", resultant_angle, turn_speed );
  
//   // if the front is clear, drive forwards
//   if( (rgr->sensors[3].range > SAFE_DIST) && // forwards
// 		(rgr->sensors[4].range > SAFE_DIST) &&
// 		(rgr->sensors[5].range > SAFE_DIST/2.0) && //
// 		(rgr->sensors[6].range > SAFE_DIST/4.0) && 
// 		(rgr->sensors[2].range > SAFE_DIST/2.0) && 
// 		(rgr->sensors[1].range > SAFE_DIST/4.0) && 
// 		(fabs( resultant_angle ) < SAFE_ANGLE) )
// 	 {
// 		forward_speed = VSPEED;
// 	 }
  
//   robot->position->SetSpeed( forward_speed, side_speed, turn_speed );

  return 0;
}
