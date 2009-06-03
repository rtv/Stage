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
} robot_t;

#define VSPEED 0.4 // meters per second
#define WGAIN 1.5 // turn speed gain
#define SAFE_DIST 0.6 // meters
#define SAFE_ANGLE 1 // radians

// forward declare
int RangerUpdate( ModelRanger* mod, robot_t* robot );

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
	
	for( std::vector<ModelRanger::Sensor>::iterator it = rgr->sensors.begin();
			 it != rgr->sensors.end();
			 ++it )
		{
			ModelRanger::Sensor& s = *it;
			dx += s.range * cos( s.pose.a );
			dy += s.range * sin( s.pose.a );
			
		//printf( "sensor %d angle= %.2f\n", s, rgr->sensors[s].pose.a );	 
	 }
  
  if( (dx == 0) || (dy == 0) )
	 return 0;
  
  assert( dy != 0 );
  assert( dx != 0 );
  
  double resultant_angle = atan2( dy, dx );
  double forward_speed = 0.0;
  double side_speed = 0.0;	   
  double turn_speed = WGAIN * resultant_angle;
  
  //printf( "resultant %.2f turn_speed %.2f\n", resultant_angle, turn_speed );

  // if the front is clear, drive forwards
  if( (rgr->sensors[0].range > SAFE_DIST) &&
		(rgr->sensors[1].range > SAFE_DIST/2.0) &&
		(rgr->sensors[2].range > SAFE_DIST/5.0) && 
		(rgr->sensors[15].range > SAFE_DIST/2.0) && 
		(rgr->sensors[14].range > SAFE_DIST/5.0) && 
		(fabs( resultant_angle ) < SAFE_ANGLE) )
	 {
		forward_speed = VSPEED;
	 }
  
  robot->position->SetSpeed( forward_speed, side_speed, turn_speed );

  return 0;
}

