/////////////////////////////////
// File: stest.c
// Desc: Stage library test program
// Created: 2004.9.15
// Author: Richard Vaughan <vaughan@sfu.ca>
// CVS: $Id: stest.cc,v 1.1.2.18 2007-11-26 06:28:16 rtv Exp $
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
  int randcount ;
  int avoidcount;
  bool obs;
} robot_t;

#define VSPEED 0.4 // meters per second
#define WGAIN 1.0 // turn speed gain
#define SAFE_DIST 0.35 // meters
#define SAFE_ANGLE 0.4 // radians

int main( int argc, char* argv[] )
{ 
  printf( "%s test program.\n", stg_package_string() );

  if( argc < 3 )
    {
      puts( "Usage: stest <worldfile> <number of robots>" );
      exit(0);
    }
  
  const int POPSIZE = atoi(argv[2] );

  // initialize libstage
  StgWorld::Init( &argc, &argv );
  //StgWorld world;
  StgWorldGui world(800, 700, "Stage Test Program");

  world.Load( argv[1] );
  
  //StgModel mod( &world, NULL, 0, "model" );
  //StgModel mod2( &world, NULL, 0, "model" );
  
  //stg_pose_t pz;
  // pz.x = 5;
  //pz.y = 0;
  //pz.z = 0;
  //pz.a = 4;//M_PI/2.0;
   //mod.SetPose( &pz );

   //pz.y = 3;
   // mod2.SetPose( &pz );

  //for( int i=0; i<10; i++ )
  //mod.AddBlockRect( i/2, 0, 0.3, 0.3 );
   
   //char* robotname = argv[2];
   
  char namebuf[256];  
  //robot_t robots[POPSIZE];
  
  robot_t* robots = new robot_t[POPSIZE];


  for( int i=0; i<POPSIZE; i++ )
    {
       robots[i].randcount = 0;
       robots[i].avoidcount = 0;
       robots[i].obs = false;
       
       char* base = "r";
       sprintf( namebuf, "%s%d", base, i );
       robots[i].position = (StgModelPosition*)world.GetModel( namebuf );
       assert(robots[i].position);
       robots[i].position->Subscribe();
       
       sprintf( namebuf, "%s%d.laser:0", base, i );
       //robots[i].laser = (StgModelLaser*)world.GetModel( namebuf );
       //assert(robots[i].laser);
       //robots[i].laser->Subscribe();

       sprintf( namebuf, "%s%d.ranger:0", base, i );
       robots[i].ranger = (StgModelRanger*)world.GetModel( namebuf );
       assert(robots[i].ranger);
       robots[i].ranger->Subscribe();
    }
   
  // start the clock
  world.Start();
  //puts( "done" );

  double newspeed = 0.0;
  double newturnrate = 0.0;

  //stg_world_set_interval_real( world, 0 );
  
  //  StgModelLaser* laser = lasers[1];

  while( world.RealTimeUpdate() )
    //   while( world.Update() )
    for( int i=0; i<POPSIZE; i++ )
	{
	  
	  StgModelRanger* rgr = robots[i].ranger;
      
      // compute the vector sum of the sonar ranges	      
      double dx=0, dy=0;
      
      int num_ranges = rgr->sensor_count;//spp[i]->GetCount();
      for( int s=0; s<num_ranges; s++ )
	{
	  //player_pose3d_t spose = spp[i]->GetPose(s);
	  double srange = rgr->sensors[s].range; //spp[i]->GetScan(s);
	  
	  dx += srange * cos( rgr->sensors[s].pose.a );
	  dy += srange * sin( rgr->sensors[s].pose.a );
	}
      
      double resultant_angle = atan2( dy, dx );
      //double resultant_magnitude = hypot( dy, dx );
      
      double forward_speed = 0.0;
      double side_speed = 0.0;	   
      double turn_speed = WGAIN * resultant_angle;
      
      int forward = num_ranges/2 -1 ;
      // if the front is clear, drive forwards
      if( (rgr->sensors[forward-1].range > SAFE_DIST) &&
	  (rgr->sensors[forward  ].range > SAFE_DIST) &&
	  (rgr->sensors[forward+1].range > SAFE_DIST) && 
	  (fabs( resultant_angle ) < SAFE_ANGLE) )
	{
	  forward_speed = VSPEED;
	}
      
      // send a command to the robot
      stg_velocity_t vel;
      vel.x = forward_speed;
      vel.y = side_speed;
      vel.z = 0;
      vel.a = turn_speed;
      
      robots[i].position->Do( forward_speed, side_speed, turn_speed );

           
      //ppp[i]->SetSpeed( forward_speed, side_speed, turn_speed );
    }

    {

      //}
      //{
      //       nothing
      //while( ! laser->DataIsFresh() )
      //if( ! world.RealTimeUpdate() )
      //break;

      
      for( int i=0; i<POPSIZE; i++ )
	{
	  //robots[i].ranger->Print( NULL );
	  
	  // get some laser data
	  size_t laser_sample_count = robots[i].laser->sample_count;      
	  stg_laser_sample_t* laserdata = robots[i].laser->samples;
      
	  if( laserdata == NULL )
	    continue;
      
      // THIS IS ADAPTED FROM PLAYER'S RANDOMWALK C++ EXAMPLE
      
      /* See if there is an obstacle in front */
      robots[i].obs = FALSE;
      for( unsigned int l = 0; l < laser_sample_count; l++)
	{
	  if(laserdata[l].range < minfrontdistance)
	    robots[i].obs = TRUE;
	}
      
      if( robots[i].obs || robots[i].avoidcount )
	{
	  newspeed = 0;
	  
	  /* once we start avoiding, continue avoiding for 2 seconds */
	  /* (we run at about 10Hz, so 20 loop iterations is about 2 sec) */
	  if( ! robots[i].avoidcount)
	    {
	      robots[i].avoidcount = 15;
	      robots[i].randcount = 0;
	      
	      // find the minimum on the left and right
	      
	      double min_left = 1e9;
	      double min_right = 1e9;
	      
	      for( unsigned int i=0; i<laser_sample_count; i++ )
		{
		  if(i>(laser_sample_count/2) && laserdata[i].range < min_left)
		    min_left = laserdata[i].range;
		  else if(i<(laser_sample_count/2) && laserdata[i].range < min_right)
		    min_right = laserdata[i].range;
		}
	      
	      if( min_left < min_right)
		newturnrate = -turnrate;
	      else
		newturnrate = turnrate;
	    }
	  
	  robots[i].avoidcount--;
	}
      else
	{
	  robots[i].avoidcount = 0;
	  newspeed = speed;
	  
	  /* update turnrate every 3 seconds */
	  if(! robots[i].randcount)
	    {
	      /* make random int tween -20 and 20 */
	      newturnrate = DTOR(rand() % 41 - 20);
	      robots[i].randcount = 50;
	    }
	  robots[i].randcount--;
	}
      
      robots[i].position->Do( newspeed, 0, newturnrate );
      //robots[i].position->Do( 0, 0, 10 );
      	}
    }  
  //#endif  


    delete[] robots;  
  exit( 0 );
}
