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

static double minfrontdistance = 0.750;
static double speed = 0.400;
static double turnrate = M_PI/3.0;

typedef struct
{
	StgModelLaser* laser;
	StgModelPosition* position;
	StgModelRanger* ranger;
} robot_t;

#define VSPEED 0.4 // meters per second
#define WGAIN 1.0 // turn speed gain
#define SAFE_DIST 0.6 // meters
#define SAFE_ANGLE 0.25 // radians

// forward declare
int RangerUpdate( StgModel* mod, robot_t* robot );

// Stage calls this when the model starts up
extern "C" int Init( StgModel* mod )
{
  robot_t* robot = new robot_t;
  robot->position = (StgModelPosition*)mod;

  // subscribe to the ranger, which we use for navigating
  robot->ranger = (StgModelRanger*)mod->GetModel( "ranger:0" );
  assert( robot->ranger );
  robot->ranger->Subscribe();
  
  // ask Stage to call into our ranger update function
  robot->ranger->AddUpdateCallback( (stg_model_callback_t)RangerUpdate, robot );

  // subscribe to the laser, though we don't use it for navigating
  robot->laser = (StgModelLaser*)mod->GetModel( "laser:0" );
  assert( robot->laser );
  //robot->laser->Subscribe();

  return 0; //ok
}

int RangerUpdate( StgModel* mod, robot_t* robot )
{
  StgModelRanger* rgr = robot->ranger;
  
  if( rgr->samples == NULL )
	 return 0;
  
  // compute the vector sum of the sonar ranges	      
  double dx=0, dy=0;
  
  for( unsigned int s=0; s< rgr->sensor_count; s++ )
	 {
		double srange = rgr->samples[s];
		
		dx += srange * cos( rgr->sensors[s].pose.a );
		dy += srange * sin( rgr->sensors[s].pose.a );
	 }
  
  if( (dx == 0) || (dy == 0) )
	 return 0;
  
  assert( dy != 0 );
  assert( dx != 0 );
  
  double resultant_angle = atan2( dy, dx );
  double forward_speed = 0.0;
  double side_speed = 0.0;	   
  double turn_speed = WGAIN * resultant_angle;
  
  int forward = rgr->sensor_count/2 -1 ;
  // if the front is clear, drive forwards
  if( (rgr->samples[forward-1] > SAFE_DIST/5.0) &&
		(rgr->samples[forward  ] > SAFE_DIST) &&
		(rgr->samples[forward+1] > SAFE_DIST/5.0) && 
		(fabs( resultant_angle ) < SAFE_ANGLE) )
	 {
		forward_speed = VSPEED;
	 }
  
  // 	// send a command to the robot
  // 	stg_velocity_t vel;
  // 	bzero(&vel,sizeof(vel));
  //         vel.x = forward_speed;
  // 	vel.y = side_speed;
  // 	vel.z = 0;
  // 	vel.a = turn_speed;
  
  //printf( "robot %s [%.2f %.2f %.2f %.2f]\n",
  //robots[i].position->Token(), vel.x, vel.y, vel.z, vel.a );
  
  //  uint32_t bcount=0;
  //stg_blobfinder_blob_t* blobs = robots[i].blobfinder->GetBlobs( &bcount );
  
  //printf( "robot %s sees %u blobs\n", robots[i].blobfinder->Token(), bcount );	       
  
  robot->position->SetSpeed( forward_speed, side_speed, turn_speed );

  return 0;
}

