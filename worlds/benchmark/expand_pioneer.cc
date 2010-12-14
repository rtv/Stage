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
	ModelRanger* ranger;
	ModelRanger* laser;
	ModelPosition* position;
} robot_t;


const double VSPEED = 0.2; // meters per second
const double WGAIN = 1.0; // turn speed gain
const double SAFE_DIST = 0.75; // meters
const double SAFE_ANGLE = 0.5; // radians


// forward declare
int RangerUpdate( ModelRanger* mod, robot_t* robot );

// Stage calls this when the model starts up
extern "C" int Init( Model* mod )
{
  robot_t* robot = new robot_t;
  robot->position = (ModelPosition*)mod;
  assert( robot->position );
  
  // subscribe to the ranger, which we use for navigating
  robot->ranger = (ModelRanger*)mod->GetUnusedModelOfType( "ranger" );
  assert( robot->ranger );
  
  // ask Stage to call into our ranger update function
  robot->ranger->AddCallback( Model::CB_UPDATE, (model_callback_t)RangerUpdate, robot );
  
  // subscribe to the laser, though we don't use it for navigating
  robot->laser = (ModelRanger*)mod->GetUnusedModelOfType( "ranger" );
  assert( robot->laser );

  // start the models updating
  robot->ranger->Subscribe();
  robot->position->Subscribe();
  //robot->laser->Subscribe();


  return 0; //ok
}

int RangerUpdate( ModelRanger* rgr, robot_t* robot )
{  	
  // compute the vector sum of the sonar ranges	      
  double dx=0, dy=0;
  
	const std::vector<ModelRanger::Sensor>& sensors = rgr->GetSensors();

  FOR_EACH( it, sensors )
	 {
		const ModelRanger::Sensor& s = *it;
		dx += s.ranges[0] * cos( s.pose.a );
		dy += s.ranges[0] * sin( s.pose.a );
		
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
  if( (sensors[3].ranges[0] > SAFE_DIST) && // forwards
		(sensors[4].ranges[0] > SAFE_DIST) &&
		(sensors[5].ranges[0] > SAFE_DIST/2.0) && //
		(sensors[6].ranges[0] > SAFE_DIST/4.0) && 
		(sensors[2].ranges[0] > SAFE_DIST/2.0) && 
		(sensors[1].ranges[0] > SAFE_DIST/4.0) && 
		(fabs( resultant_angle ) < SAFE_ANGLE) )
	 {
		forward_speed = VSPEED;
	 }
  
  robot->position->SetSpeed( forward_speed, side_speed, turn_speed );

  return 0;
}

