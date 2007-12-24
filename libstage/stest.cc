/////////////////////////////////
// File: stest.c
// Desc: Stage library test program
// Created: 2004.9.15
// Author: Richard Vaughan <vaughan@sfu.ca>
// CVS: $Id: stest.cc,v 1.1.2.21 2007-12-24 10:50:45 rtv Exp $
// License: GPL
/////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stage.hh"

double minfrontdistance = 0.750;
double speed = 0.400;
double turnrate = DTOR(60);

typedef struct
{
  StgModelLaser* laser;
  StgModelPosition* position;
  StgModelRanger* ranger;
  StgModelFiducial* fiducial;
} robot_t;

#define VSPEED 0.4 // meters per second
#define WGAIN 1.0 // turn speed gain
#define SAFE_DIST 0/8 // meters
#define SAFE_ANGLE 0.3 // radians

int main( int argc, char* argv[] )
{ 
  printf( "%s benchmarker\n", stg_package_string() );

  if( argc < 3 )
    {
      puts( "Usage: stest <worldfile> <number of robots>" );
      exit(0);
    }
  
  const int POPSIZE = atoi(argv[2] );

  // initialize libstage
  StgWorld::Init( &argc, &argv );
  //StgWorld world;
  StgWorldGui world(800, 700, "Stage Benchmark Program");

  world.Load( argv[1] );
  
  char namebuf[256];  
  robot_t* robots = new robot_t[POPSIZE];

  for( int i=0; i<POPSIZE; i++ )
    {
      //robots[i].randcount = 0;
      //robots[i].avoidcount = 0;
      //robots[i].obs = false;
       
       char* base = "r";
       sprintf( namebuf, "%s%d", base, i );
       robots[i].position = (StgModelPosition*)world.GetModel( namebuf );
       assert(robots[i].position);
       robots[i].position->Subscribe();
       
       //robots[i].laser = (StgModelLaser*)
       //robots[i].position->GetUnsubcribedModelOfType( "laser" );	 
       //assert(robots[i].laser);
       //robots[i].laser->Subscribe();

       robots[i].fiducial = (StgModelFiducial*)
	 robots[i].position->GetUnsubcribedModelOfType( "fiducial" );	 
       assert(robots[i].fiducial);
       robots[i].fiducial->Subscribe();
       
       robots[i].ranger = (StgModelRanger*)
	 robots[i].position->GetUnsubcribedModelOfType( "ranger" );
       assert(robots[i].ranger);
       robots[i].ranger->Subscribe();
    }
   
  // start the clock
  //world.Start();
  //puts( "done" );
  
  while( ! world.TestQuit() )
    if( world.RealTimeUpdate() )
      //   if( world.Update() )
      for( int i=0; i<POPSIZE; i++ )
	{
	  
	StgModelRanger* rgr = robots[i].ranger;
	
	if( rgr->samples == NULL )
	  continue;
	
	// compute the vector sum of the sonar ranges	      
	double dx=0, dy=0;
	
	for( unsigned int s=0; s< rgr->sensor_count; s++ )
	  {
	    double srange = rgr->samples[s]; 
	    
	    dx += srange * cos( rgr->sensors[s].pose.a );
	    dy += srange * sin( rgr->sensors[s].pose.a );
	  }
	
	if( dy == 0 )
	  continue;

	if( dx == 0 )
	  continue;

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
	
	// send a command to the robot
	stg_velocity_t vel;
	bzero(&vel,sizeof(vel));
	vel.x = forward_speed;
	vel.y = side_speed;
	vel.z = 0;
	vel.a = turn_speed;
	
	//printf( "robot %s x %.2f y %.2f z %.2f a %.2f\n",
	//robots[i].position->Token(), vel.x, vel.y, vel.z, vel.a );
	
	robots[i].position->SetSpeed( forward_speed, side_speed, turn_speed );
    }
  

  delete[] robots;  
  exit( 0 );
}
